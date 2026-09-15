#!/bin/sh
#
# Copyright (C) 2007 OpenWrt.org
#
# Fallback-Pfad nach dem Erstflash: "mtd fixboss firmware" schreibt
# image_length und crc32 im BOSS-Header des ersten Erase-Blocks neu.
# Im sysupgrade-Weg erledigt das bereits platform.sh (check_fixboss /
# do_fixboss). Schlaegt der Aufruf hier fehl, liefert das Skript 1;
# uci_apply_defaults laesst es dann liegen und wiederholt es beim
# naechsten Boot.
#

. /lib/functions.sh
. /lib/functions/lantiq.sh

do_fixboss() {
	if ! mtd fixboss firmware 1>/tmp/fixboss.log 2>&1; then
		logger -t fixboss "FAILED: mtd fixboss firmware, see /tmp/fixboss.log"
		return 1
	fi

	logger -t fixboss "OK: BOSS header of partition firmware rewritten"
	return 0
}

board=$(board_name)
case "$board" in
"bintec,rs353")
	do_fixboss
	;;
esac
