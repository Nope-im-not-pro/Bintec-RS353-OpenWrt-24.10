/*
 * boss.c
 *
 * Copyright (C) 2005 Mike Baker
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "mtd.h"
#include "crc32.h"


#define MAGIC_RS353 "TELDAT ClosedEyeVisual\0" // Magic string for RS353 series
#define MAGIC_RS230 "BINTEC ChaosEndDragon\0" // Magic string for RS230 series

struct boss_header {
        char magic[23];
        uint32_t fw_version;
        uint8_t image_type;
        uint32_t image_version;
        uint32_t image_length;
        uint32_t unknown1; // May contains 0X00fa0524. gzip buffer size ?
        uint32_t unknown2; // Fill with 0
        uint32_t crc32;
        uint32_t unknown3; // Fill with 0
}__attribute__ ((packed));

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

int
mtd_fixboss(const char *mtd, size_t offset, size_t data_size)
{
	size_t data_offset;
	int fd;
	struct boss_header *boss;
	char *first_block;
	char *buf;
	ssize_t res;
	size_t block_offset;
	uint32_t crc;

	if (quiet < 2)
		fprintf(stderr, "Trying to fix BOSS header in %s at 0x%x...\n", mtd, offset);

	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}
	fprintf(stderr, "MTD device %s opened, erase size : %d\n", mtd, erasesize);

	data_offset = offset + sizeof(struct boss_header);

	/*
	 * K1 - Laengen- und CRC-Vertrag mit mkbossimg.c
	 * (port/patches/000-add-mkbossimg.patch, appendfile()):
	 * image_length ist massgeblich fuer den geprueften Bereich. crc32
	 * deckt exakt image_length Bytes ab, die unmittelbar hinter dem
	 * 52-Byte-Header beginnen. Beide Felder werden big-endian
	 * gespeichert. CRC-Variante identisch auf beiden Seiten:
	 * Poly 0xedb88320, Startwert 0xFFFFFFFF, Endinvertierung
	 * (crc32buf() in mtd liefert nicht invertiert, daher das ~ unten;
	 * crc32buf() in mkbossimg.c invertiert bereits selbst).
	 *
	 * Der abgedeckte Bereich unterscheidet sich bewusst: mkbossimg
	 * deckt die gesamte Nutzlast ab, weil die Datei vor dem Flashen als
	 * Ganzes geprueft wird (Hersteller-Web-UI, Bootmon/TFTP). Zur
	 * Laufzeit ist nur der erste Erase-Block stabil: sysupgrade schreibt
	 * die gesicherte Config und jffs2 schreibt rootfs_data in dieselbe
	 * firmware-Partition, eine Pruefsumme ueber die gesamte Nutzlast
	 * waere damit schon vor dem ersten Boot ungueltig. Deshalb ist die
	 * hier geschriebene Variante die massgebliche fuer den Flash-Inhalt,
	 * und image_length wird mitgeschrieben, damit beide Seiten denselben
	 * Bereich benennen.
	 * Beleg: Fork openwrt-RS353_1, Branch openwrt-22.03-old - boss.c
	 * (Ueberschreiben seit 38f9a7d899 unveraendert) und mkbossimg.c
	 * laufen dort zusammen, Image wird in image/Makefile auf Vielfache
	 * von 131072 gepaddet, damit der Header auf einer Erase-Block-
	 * Grenze liegt.
	 *
	 * AM GERAET ZU VERIFIZIEREN: dass der BOSS-Bootmonitor den
	 * geprueften Bereich tatsaechlich aus image_length ableitet.
	 * Pruefung ohne Schreibzugriff: Originalimage aus der firmware-
	 * Partition auslesen, die 52 Header-Bytes dekodieren und
	 * image_length gegen erasesize - 52 und gegen die Partitionsgroesse
	 * vergleichen.
	 */
	if (!data_size)
		data_size = erasesize - sizeof(struct boss_header);

	block_offset = offset & ~(erasesize - 1);
	offset -= block_offset;

	if (data_offset + data_size > mtdsize) {
		fprintf(stderr, "Offset 0x%x too large, device size 0x%x\n",
			(unsigned int)(data_offset + data_size), mtdsize);
		exit(1);
	}

	first_block = malloc(erasesize);
	if (!first_block) {
		perror("malloc");
		exit(1);
	}

	res = pread(fd, first_block, erasesize, block_offset);
	if (res != erasesize) {
		perror("pread");
		exit(1);
	}

	boss = (struct boss_header *)(first_block + offset);
	boss->image_length = htonl(data_size);
	if (strncmp(MAGIC_RS353, boss->magic, strlen(MAGIC_RS353)) != 0 &&
		strncmp(MAGIC_RS230, boss->magic, strlen(MAGIC_RS230)) != 0) {
		fprintf(stderr, "Unknown or no BOSS magic found\n");
		exit(1);
	}

	fprintf(stderr, "BOSS header found ! Image Length : %08x\n", data_size);


	buf = malloc(data_size);
	if (!buf) {
		perror("malloc");
		exit(1);
	}

	res = pread(fd, buf, data_size, block_offset + sizeof(struct boss_header));
	if (res != data_size) {
		perror("pread");
		exit(1);
	}

	crc = ~crc32buf(buf, data_size);
	boss->crc32 = htonl(crc);
	if (mtd_erase_block(fd, block_offset)) {
		fprintf(stderr, "Can't erease block at 0x%x (%s)\n", block_offset, strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "New crc32: 0x%x, rewriting block\n", crc);

	if (pwrite(fd, first_block, erasesize, block_offset) != erasesize) {
		fprintf(stderr, "Error writing block (%s)\n", strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Done.\n");

	close (fd);
	free(first_block);
	free(buf);
	sync();
	return 0;

}
