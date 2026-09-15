# BUILD_HOWTO - Bintec RS353 auf OpenWrt 24.10 bauen

Schritt-fuer-Schritt-Anleitung vom leeren Rechner bis zum fertigen Image
`*-bintec_rs353-boss-image.cev`.

Jeder Schritt hat einen kopierbaren Befehl und darunter eine Zeile, woran der
Erfolg erkennbar ist. Befehle laufen auf dem Windows-Host, sofern nicht
ausdruecklich "im Container" dabeisteht.

Feste Namen, die in der ganzen Anleitung vorkommen:

| Name | Bedeutung |
|---|---|
| `openwrt-rs353-build:24.10` | Docker-Image des Buildhosts |
| `owrt-build` | Laufender Container |
| `openwrt-rs353-src` | Named Volume, im Container auf `/build` gemountet |
| `/build/openwrt` | Produktiver OpenWrt-24.10-Quellbaum |
| `/build/portcheck` | Zweitbaum zum gefahrlosen Testen des Port-Kits |
| `/build/port` | Kopie des Port-Kits im Container |

Vor dem ersten Lauf Abschnitt 9, 10 und 11 lesen. Dort stehen die Fallen, die in
diesem Projekt bereits aufgetreten sind.

---

<!-- TOC -->
## Inhalt

- [0. Voraussetzungen](#0-voraussetzungen)
- [1. Repo-Aufbau](#1-repo-aufbau)
- [2. Build-Docker-Image bauen](#2-build-docker-image-bauen)
- [3. Container starten](#3-container-starten)
- [4. Quellbaum und Feeds vorbereiten](#4-quellbaum-und-feeds-vorbereiten)
  - [4.1 OpenWrt 24.10 klonen](#41-openwrt-2410-klonen)
  - [4.2 Stand pruefen](#42-stand-pruefen)
  - [4.3 Feeds holen und einhaengen](#43-feeds-holen-und-einhaengen)
  - [4.4 Zweitbaum fuer gefahrloses Testen (empfohlen)](#44-zweitbaum-fuer-gefahrloses-testen-empfohlen)
- [5. Port-Kit anwenden](#5-port-kit-anwenden)
  - [5.1 Port-Kit in den Container kopieren](#51-port-kit-in-den-container-kopieren)
  - [5.2 Gleichstand pruefen](#52-gleichstand-pruefen)
  - [5.3 Ausfuehrbar machen](#53-ausfuehrbar-machen)
  - [5.4 Probelauf gegen den Zweitbaum](#54-probelauf-gegen-den-zweitbaum)
  - [5.5 Anwenden auf den produktiven Baum](#55-anwenden-auf-den-produktiven-baum)
  - [5.6 Kontrolle, was sich geaendert hat](#56-kontrolle-was-sich-geaendert-hat)
- [6. Profil auf das Geraet setzen](#6-profil-auf-das-geraet-setzen)
  - [6.1 Sicherung anlegen und Profil setzen](#61-sicherung-anlegen-und-profil-setzen)
  - [6.2 Profil verifizieren](#62-profil-verifizieren)
  - [6.3 Geraetenamen gegenpruefen](#63-geraetenamen-gegenpruefen)
- [7. Build starten](#7-build-starten)
  - [7.1 Parallelitaet festlegen](#71-parallelitaet-festlegen)
  - [7.2 Bauen](#72-bauen)
  - [7.3 Fortschritt verfolgen](#73-fortschritt-verfolgen)
  - [7.4 Ende feststellen](#74-ende-feststellen)
  - [7.5 Bei einem Fehlschlag: laut bauen](#75-bei-einem-fehlschlag-laut-bauen)
- [8. Ergebnis-Image finden](#8-ergebnis-image-finden)
  - [8.1 Groesse gegen das Limit pruefen](#81-groesse-gegen-das-limit-pruefen)
  - [8.2 Pruefsumme gegenlesen](#82-pruefsumme-gegenlesen)
  - [8.3 Image auf den Windows-Host holen](#83-image-auf-den-windows-host-holen)
- [9. Vor dem Flashen lesen](#9-vor-dem-flashen-lesen)
- [10. Typische Fehler und ihre Behebung](#10-typische-fehler-und-ihre-behebung)
  - [F1 - Cannot connect to the Docker daemon](#f1---cannot-connect-to-the-docker-daemon)
  - [F2 - No such container: owrt-build](#f2---no-such-container-owrt-build)
  - [F3 - You are building with the root user](#f3---you-are-building-with-the-root-user)
  - [F4 - apply.sh bricht mit FAIL &lt;name&gt; ab](#f4---applysh-bricht-mit-fail-name-ab)
  - [F5 - already applied obwohl nichts angewandt wurde](#f5---already-applied-obwohl-nichts-angewandt-wurde)
  - [F6 - Build endet mit RS353_BUILD_FAIL, Ursache unklar](#f6---build-endet-mit-rs353_build_fail-ursache-unklar)
  - [F7 - Objektdateien mit Laenge 0 nach einem harten Abbruch](#f7---objektdateien-mit-laenge-0-nach-einem-harten-abbruch)
  - [F8 - SIZE_TOO_BIG in Schritt 8.1](#f8---size_too_big-in-schritt-81)
  - [F9 - .config ist nach einem Fehlversuch durcheinander](#f9---config-ist-nach-einem-fehlversuch-durcheinander)
  - [F10 - Aenderung am Port-Kit wirkt nicht](#f10---aenderung-am-port-kit-wirkt-nicht)
  - [F11 - phase4.sh meldet PHASE4_OK, obwohl nichts gebaut wurde](#f11---phase4sh-meldet-phase4_ok-obwohl-nichts-gebaut-wurde)
  - [F12 - sh: bad substitution oder Abbruch direkt beim Start von apply.sh](#f12---sh-bad-substitution-oder-abbruch-direkt-beim-start-von-applysh)
- [11. Wiederaufnahme nach Abbruch](#11-wiederaufnahme-nach-abbruch)
  - [11.1 Sauber abbrechen](#111-sauber-abbrechen)
  - [11.2 Zustand pruefen](#112-zustand-pruefen)
  - [11.3 Fortsetzen](#113-fortsetzen)
  - [11.4 Nach einem Neustart des Rechners](#114-nach-einem-neustart-des-rechners)
  - [11.5 Alles verwerfen und neu anfangen](#115-alles-verwerfen-und-neu-anfangen)
- [12. Kurzreferenz](#12-kurzreferenz)
- [13. Linux-Host ohne Docker](#13-linux-host-ohne-docker)
  - [13.1 Ein Kommando](#131-ein-kommando)
  - [13.2 Was das Skript tut](#132-was-das-skript-tut)
  - [13.3 Grenzen](#133-grenzen)

<!-- /TOC -->

---
## 0. Voraussetzungen

Was auf dem Windows-Host vorhanden sein muss:

| Ding | Mindestanforderung | Pruefbefehl |
|---|---|---|
| Docker Desktop | laeuft, Linux-Container-Modus | `docker version` |
| Freier Plattenplatz | 40 GB im Docker-Datenbereich | `docker system df` |
| Arbeitsspeicher | 8 GB, besser 16 GB | Task-Manager |
| CPU-Kerne | 4 oder mehr | Task-Manager |
| Projektordner | `<PFAD>\OpenWRT_Update` | `dir` |

Auf dem Host wird NICHT gebaut. Alles Compilieren passiert im Container. Grund:
Windows-Dateisysteme unterscheiden Gross- und Kleinschreibung nicht, der
OpenWrt-Quellbaum enthaelt aber Dateien, die sich nur darin unterscheiden. Ein
Bind-Mount von Windows nach Linux zerstoert den Baum.

Docker ist damit reine Windows-Kruecke, keine Build-Anforderung. Auf einem
Linux-Rechner faellt dieser Grund weg, dort wird ohne Docker direkt auf dem Host
gebaut. Dieser Weg steht in Abschnitt 13 und ist mit einem Skript abgedeckt.

```bash
docker version
```

Erfolg: Es erscheinen zwei Bloecke, `Client` und `Server`, und im Server-Block
steht `OS/Arch: linux/amd64`. Fehlt der Server-Block, laeuft der Docker-Dienst
nicht.

---

## 1. Repo-Aufbau

So ist der Projektordner aufgebaut. Nichts davon muss von Hand erzeugt werden,
es ist bereits vorhanden.

```
<PFAD>\OpenWRT_Update\
+-- docker\
|   \-- Dockerfile                  Buildhost-Definition (Debian bookworm)
+-- port\                           das "Port-Kit": alles Geraetespezifische
|   +-- apply.sh                    traegt das Port-Kit in einen Quellbaum ein
|   +-- phase4.sh                   Profil setzen, bauen, pruefen (siehe F11)
|   +-- patches\                    13 Diffs/Patches gegen 24.10
|   \-- tree\                       4 komplett neue Dateien
|       +-- package\system\mtd\src\boss.c
|       +-- target\linux\generic\files\drivers\mtd\mtdsplit\mtdsplit_bintec.c
|       +-- target\linux\lantiq\base-files\etc\uci-defaults\09_fix_crc.sh
|       \-- target\linux\lantiq\files\arch\mips\boot\dts\lantiq\vr9_bintec_rs353.dts
+-- UMSETZUNGSPLAN.md               verbindlicher Plan, Phasen 0-6
+-- PROCESS_STEPS.md                Live-Mitschrift, was wann lief
+-- BUILD_HOWTO.md                  diese Datei
\-- README.md, CHANGELOG.md, INFRA.md, MVC.md, ERKLAERUNG.md
```

Was das Port-Kit inhaltlich tut, in einem Satz: es fuegt dem OpenWrt-Baum das
Geraet `bintec_rs353` hinzu - Device Tree, Flash-Aufteilung, das
BOSS-Containerformat des Herstellers und die Netzwerkkonfiguration.

```bash
dir <PFAD>\OpenWRT_Update\port\patches
```

Erfolg: Es werden 13 Dateien gelistet - elf `*.diff`, dazu
`000-add-mkbossimg.patch` und
`0999-MTD-lantiq-flash-map-add-ebu-endianness-check.patch`.

---

## 2. Build-Docker-Image bauen

Das Image bringt alle Compiler und Hilfsprogramme mit, die OpenWrt braucht.
Einmal bauen, danach wiederverwenden.

```bash
docker build -t openwrt-rs353-build:24.10 --build-arg UID=1000 --build-arg GID=1000 <PFAD>\OpenWRT_Update\docker
```

Erfolg: Letzte Zeile lautet `naming to docker.io/library/openwrt-rs353-build:24.10`
oder `Successfully tagged openwrt-rs353-build:24.10`. Dauer beim ersten Mal 5
bis 15 Minuten.

Gegenprobe, falls unklar ist ob das Image schon existiert:

```bash
docker images openwrt-rs353-build:24.10
```

Erfolg: Eine Zeile mit `TAG 24.10` und einer Groesse um 1.6 GB.

---

## 3. Container starten

Der Container bekommt ein Named Volume auf `/build`. Das Volume liegt im
Linux-Dateisystem von Docker, nicht auf `C:`. Genau das ist der Punkt: kein
Windows-Dateisystem im Quellbaum.

```bash
docker run -d --name owrt-build -v openwrt-rs353-src:/build -w /build openwrt-rs353-build:24.10 sleep infinity
```

Erfolg: Es wird eine 64-stellige Container-ID ausgegeben.

```bash
docker ps --filter name=owrt-build
```

Erfolg: Eine Zeile mit `owrt-build` und Status `Up ...`.

Meldet Docker `Conflict. The container name "/owrt-build" is already in use`,
existiert der Container bereits. Dann nur starten statt neu anlegen:

```bash
docker start owrt-build
```

Erfolg: Die Ausgabe ist die Zeile `owrt-build`.

Mit dem Container arbeiten: jeder Befehl "im Container" folgt diesem Muster.

```bash
docker exec -u build owrt-build bash -lc "whoami; pwd"
```

Erfolg: Zwei Zeilen, `build` und `/build`. Steht dort `root`, fehlt das
`-u build`; ohne Nutzerwechsel bricht OpenWrt den Build mit einer Root-Warnung
ab.

---

## 4. Quellbaum und Feeds vorbereiten

### 4.1 OpenWrt 24.10 klonen

```bash
docker exec -u build owrt-build bash -lc "cd /build && git clone --branch openwrt-24.10 https://git.openwrt.org/openwrt/openwrt.git openwrt"
```

Erfolg: Letzte Zeile `Resolving deltas: 100% (...)`, danach existiert
`/build/openwrt`. Existiert der Baum schon, bricht git mit
`destination path 'openwrt' already exists` ab - das ist ab dem zweiten Lauf der
Normalfall, Schritt ueberspringen.

### 4.2 Stand pruefen

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && git log --oneline -1"
```

Erfolg: Eine Zeile wie `a1ea57b kernel: bump 6.6 to 6.6.151`. Der genaue Hash
darf abweichen; entscheidend ist ein Commit aus dem Zweig `openwrt-24.10`.

### 4.3 Feeds holen und einhaengen

Feeds sind die Paketquellen (LuCI-Weboberflaeche, Zusatzpakete, Routing).

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && ./scripts/feeds update -a && ./scripts/feeds install -a"
```

Erfolg: Die Ausgabe endet ohne `ERROR:`.

```bash
docker exec -u build owrt-build bash -lc "ls /build/openwrt/package/feeds"
```

Erfolg: Genau die vier Ordner `base`, `luci`, `packages`, `routing`.

### 4.4 Zweitbaum fuer gefahrloses Testen (empfohlen)

Das Port-Kit zuerst gegen eine Kopie fahren. Geht dabei etwas schief, bleibt der
produktive Baum unberuehrt.

```bash
docker exec -u build owrt-build bash -lc "cd /build && [ -d portcheck ] || git clone --shared /build/openwrt portcheck"
```

Erfolg: `/build/portcheck` existiert. Bei bereits vorhandenem Ordner gibt der
Befehl nichts aus - ebenfalls Erfolg.

---

## 5. Port-Kit anwenden

### 5.1 Port-Kit in den Container kopieren

Wichtig zu verstehen: Der Container hat KEINEN Zugriff auf `C:`. Es gibt keinen
Bind-Mount. Das Port-Kit muss aktiv hineinkopiert werden - und zwar nach jeder
Aenderung auf dem Windows-Host erneut. Wer das vergisst, baut stillschweigend
mit einem alten Stand.

```bash
docker cp <PFAD>\OpenWRT_Update\port owrt-build:/build/port
```

Erfolg: Kein Fehlertext.

```bash
docker exec -u build owrt-build bash -lc "ls -1 /build/port"
```

Erfolg: Es erscheinen `apply.sh`, `phase4.sh`, `patches`, `tree`. Fehlt
`phase4.sh`, wurde der Kopierschritt nicht ausgefuehrt - dann 5.1 wiederholen,
bevor es weitergeht.

### 5.2 Gleichstand pruefen

Vergleicht die Pruefsumme beider Seiten. Nur wenn sie uebereinstimmen, baut der
Container das, was auf dem Host liegt.

```bash
docker exec -u build owrt-build bash -lc "md5sum /build/port/apply.sh"
```

Erfolg: Der ausgegebene Hash stimmt mit dem des Hosts ueberein.

```bash
certutil -hashfile <PFAD>\OpenWRT_Update\port\apply.sh MD5
```

Erfolg: Die mittlere Zeile enthaelt denselben Hash wie oben (Gross- und
Kleinschreibung ignorieren).

### 5.3 Ausfuehrbar machen

`docker cp` uebertraegt die Windows-Rechte nicht sinnvoll.

```bash
docker exec -u build owrt-build bash -lc "chmod 755 /build/port/apply.sh /build/port/phase4.sh"
```

Erfolg: Keine Ausgabe.

### 5.4 Probelauf gegen den Zweitbaum

`apply.sh` ist ein Bash-Skript mit `set -euo pipefail` und muss mit `bash`
gestartet werden, nicht mit `sh`.

```bash
docker exec -u build owrt-build bash -lc "bash /build/port/apply.sh /build/portcheck"
```

Erfolg: Die letzte Zeile lautet `PORT_APPLY_OK`. Dazwischen steht fuer jeden
Patch eine Zeile `applied ...`. Bricht es mit `FAIL <name>` ab, siehe F4.

### 5.5 Anwenden auf den produktiven Baum

Erst ausfuehren, wenn 5.4 `PORT_APPLY_OK` geliefert hat.

```bash
docker exec -u build owrt-build bash -lc "bash /build/port/apply.sh /build/openwrt"
```

Erfolg: Wieder `PORT_APPLY_OK` als letzte Zeile.

### 5.6 Kontrolle, was sich geaendert hat

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && git status --porcelain"
```

Erfolg: Rund 18 Zeilen - etwa zwoelf mit `M` (geaenderte Dateien) und sechs mit
`??` (neue Dateien, darunter `vr9_bintec_rs353.dts`, `boss.c`,
`mtdsplit_bintec.c`, `09_fix_crc.sh`, die beiden kopierten Patches). Ist die
Liste leer, wurde nichts angewandt.

Zur Wiederholbarkeit: `apply.sh` erkennt bereits angewandte Patches und meldet
dann `already applied ...`. Ein zweiter Lauf ist daher unschaedlich. Die
Erkennung arbeitet aber mit einer Textsuche, nicht mit einer echten
Patch-Pruefung - Gegenprobe siehe F5.

---

## 6. Profil auf das Geraet setzen

### 6.1 Sicherung anlegen und Profil setzen

Der Befehl loescht gezielt die bisherigen Geraete- und Profilzeilen aus der
`.config` und traegt den RS353 ein. Das `cp .config .config.bak` davor ist die
Sicherung; ohne sie ist ein Fehlversuch nur durch kompletten Neuaufbau der
Konfiguration zu reparieren.

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && cp .config .config.bak 2>/dev/null; sed -i '/^CONFIG_TARGET_lantiq_xrx200_DEVICE_/d;/^CONFIG_TARGET_PROFILE=/d' .config 2>/dev/null; printf 'CONFIG_TARGET_lantiq=y\nCONFIG_TARGET_lantiq_xrx200=y\nCONFIG_TARGET_lantiq_xrx200_DEVICE_bintec_rs353=y\n' >> .config && make defconfig"
```

Erfolg: Der Befehl endet ohne Fehler, `make defconfig` schreibt eine bereinigte
`.config`.

### 6.2 Profil verifizieren

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && grep -E '^CONFIG_TARGET_(PROFILE|lantiq_xrx200_DEVICE_bintec_rs353)' .config"
```

Erfolg: Genau zwei Zeilen:

```
CONFIG_TARGET_PROFILE="DEVICE_bintec_rs353"
CONFIG_TARGET_lantiq_xrx200_DEVICE_bintec_rs353=y
```

Fehlt die `PROFILE`-Zeile, hat `make defconfig` das Geraet nicht gefunden - dann
wurde das Port-Kit nicht vollstaendig angewandt, zurueck zu Schritt 5.

### 6.3 Geraetenamen gegenpruefen

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && make -s target/info 2>/dev/null | grep -A4 bintec_rs353 | head -12"
```

Erfolg: Es erscheint der Eintrag mit `Bintec RS353` und den unterstuetzten
Geraetenamen `bintec,rs353` und `rs353`.

---

## 7. Build starten

### 7.1 Parallelitaet festlegen

`-j` gibt an, wie viele Compiler-Prozesse gleichzeitig laufen. Mehr ist nicht
automatisch besser: In diesem Projekt hat `-j8` den Host so belastet, dass der
Build hart beendet werden musste und dabei acht Objektdateien mit Laenge 0
zurueckblieben (siehe F7). Empfehlung: `-j4`, und nur erhoehen, wenn der Rechner
sichtbar Luft hat.

### 7.2 Bauen

Der Build laeuft lange (erstmalig 1 bis 3 Stunden), deshalb im Hintergrund mit
Protokoll in eine Datei. Die Endmarke wird von `make` selbst abhaengig gemacht,
nicht von einer Pipe.

```bash
docker exec -u build -d owrt-build bash -lc "cd /build/openwrt && { make -j4 && echo RS353_BUILD_OK || echo RS353_BUILD_FAIL; } > /build/rs353-build.log 2>&1"
```

Erfolg: Der Befehl kehrt sofort zurueck (`-d` = detached). Der Build laeuft
weiter, auch wenn das Terminal geschlossen wird.

### 7.3 Fortschritt verfolgen

```bash
docker exec -u build owrt-build bash -lc "tail -20 /build/rs353-build.log"
```

Erfolg: Es erscheinen `make[n] -C ...`-Zeilen. Solange sich die Ausgabe bei
Wiederholung aendert, laeuft der Build.

### 7.4 Ende feststellen

```bash
docker exec -u build owrt-build bash -lc "grep -E 'RS353_BUILD_(OK|FAIL)' /build/rs353-build.log"
```

Erfolg: `RS353_BUILD_OK`. Erscheint nichts, laeuft der Build noch. Erscheint
`RS353_BUILD_FAIL`, weiter mit Abschnitt 10.

Warum die Endmarke wichtig ist: Der Exit-Status eines Builds geht verloren,
sobald die Ausgabe durch eine Pipe laeuft (etwa `make | tail`). Dann meldet die
Pipe Erfolg, obwohl der Build gescheitert ist. Die Marke `RS353_BUILD_OK` wird
nur geschrieben, wenn `make` selbst erfolgreich war. Sie ist die einzige
verlaessliche Aussage.

### 7.5 Bei einem Fehlschlag: laut bauen

Der parallele Build vermischt die Ausgaben. Zum Diagnostizieren einmal seriell
und ausfuehrlich nachfahren - das bricht an derselben Stelle ab, dann aber
lesbar.

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && make -j1 V=s 2>&1 | tail -80"
```

Erfolg im Sinne der Diagnose: Die letzten Zeilen nennen Datei und Zeile des
eigentlichen Fehlers.

---

## 8. Ergebnis-Image finden

```bash
docker exec -u build owrt-build bash -lc "ls -l /build/openwrt/bin/targets/lantiq/xrx200/ | grep -i bintec"
```

Erfolg: Eine Datei, deren Name auf `-boss-image.cev` endet.

### 8.1 Groesse gegen das Limit pruefen

Das Flash-Fach fuer die Firmware ist begrenzt. Ein zu grosses Image passt nicht
und darf nicht auf das Geraet.

```bash
docker exec -u build owrt-build bash -lc 'IMG=$(ls /build/openwrt/bin/targets/lantiq/xrx200/*bintec*boss-image.cev | head -n1); SZ=$(stat -c %s "$IMG"); echo "$IMG"; echo "bytes=$SZ limit=$((31232*1024))"; [ "$SZ" -le $((31232*1024)) ] && echo SIZE_OK || echo SIZE_TOO_BIG'
```

Erfolg: Letzte Zeile `SIZE_OK`.

### 8.2 Pruefsumme gegenlesen

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt/bin/targets/lantiq/xrx200 && sha256sum -c sha256sums 2>/dev/null | grep -i bintec"
```

Erfolg: Die Zeile endet mit `: OK`.

### 8.3 Image auf den Windows-Host holen

```bash
docker cp owrt-build:/build/openwrt/bin/targets/lantiq/xrx200 <PFAD>\OpenWRT_Update\out
```

Erfolg: Der Ordner `out\xrx200` existiert auf dem Host und enthaelt die
`.cev`-Datei sowie `sha256sums`.

---

## 9. Vor dem Flashen lesen

Das Image ist gebaut - damit ist es noch nicht geraetereif. Die kritischen
Befunde des statischen Reviews (K1, K2, K3) sind im Port-Kit umgesetzt. Was
bleibt, sind Vorbehalte, die nur am Geraet selbst entschieden werden koennen.
Sie stehen hier, damit niemand ahnungslos flasht.

| Punkt | Worum es geht | Quelle | Was zu tun ist |
|---|---|---|---|
| Header-Semantik | Beide Seiten folgen seit K1 demselben Vertrag: `image_length` fuehrt den geprueften Bereich mit, `crc32` deckt genau diese Bytes hinter dem 52-Byte-Header ab, beide big-endian. Der Umfang unterscheidet sich bewusst - `mkbossimg` die ganze Nutzlast (die Datei wird vor dem Flashen als Ganzes geprueft), `mtd fixboss` nach dem Flashen nur den ersten Erase-Block (jffs2/rootfs_data schreiben in dieselbe Partition). Nicht belegt bleibt, ob der BOSS-Bootmonitor den geprueften Bereich tatsaechlich aus `image_length` ableitet. | `port/tree/package/system/mtd/src/boss.c:80-114` (Vertrag), `:140` und `:162-163`; `port/patches/000-add-mkbossimg.patch:150-171` (Vertrag), `:181-182` | Vor dem ersten Flashen den Header eines funktionierenden Herstellerimages auslesen und `image_length` gegen `erasesize - 52` sowie gegen die Partitionsgroesse vergleichen. Erst danach flashen. |
| Erstflash-Weg | Der sysupgrade-Pfad ruft `mtd fixboss` aus der noch laufenden ALTEN Firmware auf. Eine Herstellerfirmware kennt dieses Kommando nicht. Seit K2 wird die Verfuegbarkeit VOR dem Schreiben geprueft (`check_fixboss`) und der Rueckgabewert danach ausgewertet (`do_fixboss`), beides mit Abbruch und lauter Meldung auf stderr. Der Weg scheitert damit sichtbar statt still - lauffaehig fuer den Erstflash aus Herstellerfirmware wird er dadurch nicht. | `port/patches/src_target_linux_lantiq_xrx200_base-files_lib_upgrade_platform.sh.diff:9-13` (Aufrufstelle), `:24-36` (`check_fixboss`), `:43-51` (`do_fixboss`) | Den ersten Flash ueber den Bootmonitor an der seriellen Konsole fahren, nicht ueber sysupgrade. |
| Fluechtiges Protokoll | Das Startskript wertet seit H1 den Rueckgabewert von `mtd fixboss` aus: bei Fehlschlag gibt es 1 zurueck, `uci_apply_defaults` laesst es dann liegen und wiederholt es beim naechsten Boot. Die Ausgabe landet in `/tmp/fixboss.log`, zusaetzlich meldet `logger` Erfolg oder Fehlschlag ins Systemlog. Beides liegt im RAM. | `port/tree/target/linux/lantiq/base-files/etc/uci-defaults/09_fix_crc.sh:16-24` | Nach dem ersten Boot sofort `logread` und `/tmp/fixboss.log` ansehen, solange das Geraet noch laeuft. `/tmp` ist fluechtig - nach einem Neustart ist das Protokoll weg. |

Serielle Konsole bereitlegen (3,3 V TTL, 115200 8N1). Ohne sie gibt es keinen
Rueckweg, wenn das Geraet nicht mehr startet.

Zwei weitere Punkte aus dem Review, die kein Bauhindernis sind, aber vor dem
Flashen bekannt sein sollten:

- USB-Stromversorgung: Die in Kernel 6.6 entfallene Eigenschaft
  `enable-active-low` ist entfernt, die Schaltrichtung steht jetzt im GPIO-Flag
  selbst (`gpio = <&gpio 14 GPIO_ACTIVE_LOW>;`, `vr9_bintec_rs353.dts:143`,
  Vermerk in `:142`). Ob ACTIVE_LOW der realen Beschaltung entspricht, ist nur
  am Geraet messbar. Folge im schlechtesten Fall: USB-Anschluss bleibt tot oder
  ist dauerhaft bestromt. Kein Brick-Risiko.
- Partitionsgroesse: Die Firmware-Partition im Device Tree ist 32000 KiB gross
  (`vr9_bintec_rs353.dts:67`), das Limit im Bauplan steht auf 31232 KiB
  (`port/apply.sh:151`). Die Herkunft des Werts ist dort dokumentiert
  (`port/apply.sh:145-150`): uebernommen aus dem Fork `openwrt-RS353_1`, Branch
  `openwrt-22.03-old`, ohne RS353-spezifische Herleitung. Die reale Obergrenze
  des BOSS-Bootmonitors ist am Geraet zu verifizieren. Das Limit nicht
  eigenmaechtig anheben.

---

## 10. Typische Fehler und ihre Behebung

### F1 - `Cannot connect to the Docker daemon`

Docker Desktop laeuft nicht.

```bash
docker info
```

Erfolg: Ein langer Block mit `Server Version:`. Kommt stattdessen die
Fehlermeldung, Docker Desktop starten und warten, bis das Wal-Symbol ruhig ist.

### F2 - `No such container: owrt-build`

Der Container wurde entfernt oder nie angelegt.

```bash
docker ps -a --filter name=owrt-build
```

Erfolg: Der Container ist gelistet und kann mit `docker start owrt-build`
gestartet werden. Ist die Liste leer, mit dem `docker run` aus Schritt 3 neu
anlegen; das Volume `openwrt-rs353-src` und damit der Quellbaum bleiben dabei
erhalten.

### F3 - `You are building with the root user`

Das `-u build` wurde vergessen. Alle Befehle im Container brauchen es.

```bash
docker exec -u build owrt-build bash -lc "id -un"
```

Erfolg: Ausgabe `build`.

### F4 - `apply.sh` bricht mit `FAIL <name>` ab

Ein Patch passt nicht mehr auf den Quellbaum, meist weil der Baum bereits halb
bearbeitet ist oder sich der OpenWrt-Zweig bewegt hat.

Zustand sichtbar machen:

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && git status --porcelain"
```

Erfolg der Diagnose: Die Liste zeigt, welche Dateien bereits angefasst wurden.

Sauberer Neuanfang des Quellbaums - verwirft alle lokalen Aenderungen, auch die
des Port-Kits, aber nicht den Build-Fortschritt:

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && git checkout -- . && git clean -fd"
```

Erfolg: `git status --porcelain` gibt danach nichts mehr aus. Anschliessend
Schritt 5 wiederholen.

### F5 - `already applied` obwohl nichts angewandt wurde

`apply.sh` erkennt einen bereits angewandten Patch daran, dass es die erste
eingefuegte Zeile im Zielfile wiederfindet (`port/apply.sh:37-39`). Kommt diese
Zeichenfolge aus anderem Grund vor, wird der Patch stillschweigend
uebersprungen, und der Fehler faellt erst beim Bauen oder gar nicht auf.

Gegenprobe - diese fuenf Marken muessen nach einem vollstaendigen Apply
vorhanden sein:

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && grep -c CMD_FIXBOSS package/system/mtd/src/mtd.c; grep -c MTD_SPLIT_BINTEC_FW target/linux/generic/files/drivers/mtd/mtdsplit/Kconfig; grep -c 'obj.lantiq = boss.o' package/system/mtd/src/Makefile; grep -c 'Build/rs353-cev' target/linux/lantiq/image/Makefile; grep -c 'bintec,rs353' target/linux/lantiq/xrx200/base-files/etc/board.d/02_network"
```

Erfolg: Fuenf Zahlen, jede groesser als 0. Steht irgendwo `0`, fehlt dieser
Teil - dann F4 (sauberer Neuanfang) und Schritt 5 erneut.

### F6 - Build endet mit `RS353_BUILD_FAIL`, Ursache unklar

Erst die echte Fehlermeldung holen, nicht raten.

```bash
docker exec -u build owrt-build bash -lc "grep -n -iE 'error|Error [0-9]+' /build/rs353-build.log | tail -20"
```

Erfolg: Die Trefferliste nennt die abbrechende Datei. Danach mit 7.5 seriell
nachfahren.

### F7 - Objektdateien mit Laenge 0 nach einem harten Abbruch

Wurde ein Build mit `kill -9` oder durch Abschalten des Containers beendet,
koennen halb geschriebene Objektdateien liegen bleiben. Der naechste Build
scheitert dann an scheinbar zufaelliger Stelle - genau das ist in diesem Projekt
in `toolchain/gcc/initial` passiert.

Aufspueren:

```bash
docker exec -u build owrt-build bash -lc "find /build/openwrt/build_dir -name '*.o' -size 0 | head -20"
```

Erfolg: Keine Ausgabe. Kommen Pfade, das betroffene Paket zuruecksetzen -
Beispiel fuer den Compiler selbst:

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && make toolchain/gcc/initial/clean"
```

Erfolg: Der Befehl endet ohne Fehler; die 0-Byte-Suche oben liefert danach
nichts mehr.

Merksatz: Builds nie mit `kill -9` beenden. Abschnitt 11 zeigt den sauberen Weg.

### F8 - `SIZE_TOO_BIG` in Schritt 8.1

Das Image ueberschreitet 31232 KiB. Ursache sind fast immer zusaetzlich
ausgewaehlte Pakete.

```bash
docker exec -u build owrt-build bash -lc "ls -lS /build/openwrt/bin/targets/lantiq/xrx200/packages | head -20"
```

Erfolg der Diagnose: Die groessten Pakete stehen oben. Auswahl ueber
`make menuconfig` reduzieren, danach Schritt 7 wiederholen. Das Limit selbst
nicht anheben - Begruendung in Abschnitt 9.

### F9 - `.config` ist nach einem Fehlversuch durcheinander

Schritt 6.1 loescht Zeilen aus der `.config`. Geht dabei etwas schief, hilft die
dort angelegte Sicherung:

```bash
docker exec -u build owrt-build bash -lc "cd /build/openwrt && cp .config.bak .config && make defconfig"
```

Erfolg: `grep CONFIG_TARGET_PROFILE .config` zeigt wieder einen Wert. Existiert
keine `.config.bak`, die `.config` loeschen und Schritt 6.1 komplett neu fahren.

### F10 - Aenderung am Port-Kit wirkt nicht

Klassiker. Der Container arbeitet mit einer Kopie, nicht mit dem Windows-Ordner.

```bash
docker exec -u build owrt-build bash -lc "md5sum /build/port/apply.sh"
```

Erfolg: Der Hash entspricht dem Host-Stand aus 5.2. Weicht er ab, Schritt 5.1
wiederholen - und daran denken, dass danach auch 5.5 noch einmal noetig ist.

### F11 - `phase4.sh` meldet `PHASE4_OK`, obwohl nichts gebaut wurde

`port/phase4.sh` fuehrt die Schritte 5 bis 8 in einem Rutsch aus. Die frueher
hier genannten Schwaechen sind behoben:

- `phase4.sh:35` schreibt `make` ohne Pipe in das Log und setzt den Endmarker
  `RS353_BUILD_OK` / `RS353_BUILD_FAIL` selbst, `phase4.sh:37` prueft ihn. Der
  Exit-Status geht nicht mehr in einer Pipe verloren.
- `phase4.sh:16` legt vor dem `sed` auf der `.config` die Sicherung
  `.config.bak` an. Rueckweg: `cp .config.bak .config`.
- `phase4.sh:30-34` setzt die Zeitmarke `/tmp/rs353-build.stamp`,
  `phase4.sh:40-42` sucht nur Images, die juenger sind als diese Marke, und
  bricht ab, wenn keines gefunden wurde. Ein altes Artefakt besteht die
  Groessenpruefung nicht mehr.

Massgeblich fuer den Build-Erfolg bleibt trotzdem die Marke aus 7.4 im Log.
Wer sicher gehen will, loescht vor dem Lauf die alten Artefakte:

```bash
docker exec -u build owrt-build bash -lc "rm -f /build/openwrt/bin/targets/lantiq/xrx200/*bintec*"
```

Erfolg: Keine Ausgabe; die Image-Suche aus Schritt 8 liefert danach nichts mehr,
bis neu gebaut wurde. `PHASE4_OK` wird nur erreicht, wenn
`grep -q "^RS353_BUILD_OK$"` in `phase4.sh:37` zugeschlagen hat und ein frisches
Image gefunden wurde; die Marke aus 7.4 in `/build/rs353-build.log` bleibt der
Beleg.

### F12 - `sh: bad substitution` oder Abbruch direkt beim Start von apply.sh

`apply.sh` ist ein Bash-Skript. Wird es mit `sh` gestartet, scheitern
`set -o pipefail` und andere Bash-Konstrukte.

```bash
docker exec -u build owrt-build bash -lc "head -1 /build/port/apply.sh"
```

Erfolg: Ausgabe `#!/bin/bash`. Aufruf immer mit `bash /build/port/apply.sh ...`
wie in 5.4 und 5.5.

---

## 11. Wiederaufnahme nach Abbruch

OpenWrt-Builds sind fortsetzbar. Fertige Teile werden nicht neu gebaut.

### 11.1 Sauber abbrechen

Nie `kill -9`, sonst F7. `SIGTERM` gibt `make` die Gelegenheit, angefangene
Zieldateien selbst zu entfernen.

```bash
docker exec -u build owrt-build bash -lc "pkill -TERM -f 'make -j' ; sleep 3 ; pgrep -c -f 'make -j' || echo NO_MAKE_RUNNING"
```

Erfolg: Ausgabe `NO_MAKE_RUNNING` oder die Zahl `0`.

### 11.2 Zustand pruefen

```bash
docker exec -u build owrt-build bash -lc "find /build/openwrt/build_dir /build/openwrt/staging_dir -name '*.o' -size 0 2>/dev/null | grep -v cmake | head"
```

Erfolg: Keine Ausgabe. Kommen Treffer, das betroffene Paket nach F7 saeubern.
Treffer unterhalb von `cmake`-Testverzeichnissen sind harmlos und werden hier
bewusst ausgefiltert.

### 11.3 Fortsetzen

```bash
docker exec -u build -d owrt-build bash -lc "cd /build/openwrt && { make -j4 && echo RS353_BUILD_OK || echo RS353_BUILD_FAIL; } >> /build/rs353-build.log 2>&1"
```

Erfolg: Der Befehl kehrt sofort zurueck; `tail -5 /build/rs353-build.log` zeigt
nach kurzer Zeit wieder laufende `make`-Zeilen. Bereits fertige Pakete werden
uebersprungen, der Build setzt an der Abbruchstelle an.

### 11.4 Nach einem Neustart des Rechners

Der Container ist gestoppt, Volume und Quellbaum sind unversehrt.

```bash
docker start owrt-build
```

Erfolg: Ausgabe `owrt-build`. Danach direkt mit 11.3 fortsetzen - die Schritte 2
bis 6 entfallen, solange am Port-Kit nichts geaendert wurde.

### 11.5 Alles verwerfen und neu anfangen

Nur im Notfall. Loescht den kompletten Quellbaum samt Build-Fortschritt; der
naechste Durchlauf beginnt wieder bei Stunde null.

```bash
docker rm -f owrt-build; docker volume rm openwrt-rs353-src
```

Erfolg: Beide Befehle geben den jeweiligen Namen aus. Danach bei Schritt 3
wieder einsteigen.

---

## 12. Kurzreferenz

Der komplette Weg in der richtigen Reihenfolge, wenn alles bereits einmal lief:

| Schritt | Befehl (verkuerzt) | Erfolgsmarke |
|---|---|---|
| 3 | `docker start owrt-build` | `owrt-build` |
| 5.1 | `docker cp ...\port owrt-build:/build/port` | keine Fehlermeldung |
| 5.2 | `md5sum /build/port/apply.sh` | Hash gleich Host |
| 5.3 | `chmod 755 /build/port/*.sh` | keine Ausgabe |
| 5.5 | `bash /build/port/apply.sh /build/openwrt` | `PORT_APPLY_OK` |
| 6.2 | `grep CONFIG_TARGET_PROFILE .config` | `"DEVICE_bintec_rs353"` |
| 7.2 | `make -j4` im Hintergrund, Log `/build/rs353-build.log` | - |
| 7.4 | `grep RS353_BUILD_ /build/rs353-build.log` | `RS353_BUILD_OK` |
| 8.1 | Groessenpruefung gegen 31232k | `SIZE_OK` |
| 8.3 | `docker cp ... C:\...\out` | Datei liegt auf dem Host |
| 9 | Header und Flash-Weg pruefen | serielle Konsole bereit |

---

## 13. Linux-Host ohne Docker

Auf einem Linux-Rechner entfaellt der einzige Grund fuer den Container
(Abschnitt 0). Der Build laeuft direkt auf dem Host. Der Container-Inhalt
besteht nur aus Distro-Paketen und einem Nicht-root-Benutzer, siehe
`docker/Dockerfile`.

### 13.1 Ein Kommando

Im Projektordner:

```bash
bash build_rs353_linux.sh
```

Erfolg: Das Skript endet mit dem Block `FERTIG.` und nennt den Pfad des Images
unter `out/`. Fehlschlag: Es endet mit `FEHLER (<Schritt>)` und nennt das
Protokoll und den passenden Abschnitt dieser Anleitung.

Optionen:

| Option | Wirkung | Vorgabe |
|---|---|---|
| `--workdir PFAD` | Arbeitsordner fuer Quellbaum und Log | `$HOME/openwrt-rs353-build` |
| `--jobs N` | Parallele Compiler-Prozesse | `4` |
| `--skip-deps` | Paketinstallation ueberspringen | aus |
| `-h`, `--help` | Hilfe | - |

### 13.2 Was das Skript tut

Zehn Schritte, jeder mit eigener Erfolgspruefung. Die Zuordnung zu den
Docker-Abschnitten dieser Anleitung:

| Schritt | Inhalt | entspricht |
|---|---|---|
| 1 | Nicht-root, Projektordner vollstaendig | F3, Abschnitt 1 |
| 2 | Pakete per `apt-get` installieren | `docker/Dockerfile` |
| 3 | Gross-/Kleinschreibung und 40 GB Platz pruefen | Abschnitt 0 |
| 4 | `git clone --branch openwrt-24.10` | 4.1, 4.2 |
| 5 | `feeds update -a` und `install -a` | 4.3 |
| 6 | `bash port/apply.sh`, danach fuenf Pruefmarken | 5.5, F5 |
| 7 | Profil setzen, `.config.bak` sichern | 6.1, 6.2, F9 |
| 8 | `make -j4` in Datei, Endmarker statt Pipe | 7.2, 7.4 |
| 9 | Image juenger als Startmarke, Groesse gegen 31232k | 8.1, F11 |
| 10 | Kopie nach `out/`, Warnung vor dem Flashen | 8.3, Abschnitt 9 |

Bewusste Festlegungen: `-j4` statt `nproc`, weil hoehere Parallelitaet in diesem
Projekt bereits zu Speichermangel gefuehrt hat (F7). `make` schreibt ohne Pipe in
eine Datei, sonst geht der Exit-Status verloren (7.4). Das gefundene Image muss
juenger sein als eine vor dem Build gesetzte Marke, sonst wird ein Altbestand
als Erfolg gemeldet (F11).

### 13.3 Grenzen

- Nur Debian und Ubuntu installieren Pakete selbsttaetig. Andere Distributionen:
  Das Skript nennt die Paketliste und bricht ab, danach `--skip-deps`.
- Der Arbeitsordner muss im Linux-Dateisystem liegen. Eingebundene
  Windows-Laufwerke (NTFS, exFAT) fallen im Test in Schritt 3 durch.
- `port/apply.sh`, `port/phase4.sh` und `port/tree/.../09_fix_crc.sh` sind in
  `.gitignore` von der `*.sh`-Regel ausgenommen und liegen im Repo. Ein reiner
  GitHub-Klon kann bauen.
- Zeilenenden: `.gitattributes` setzt `* text=auto eol=lf`. Wer aelter geklonte
  Kopien mit CRLF herumliegen hat, klont neu. Ein Skript mit CRLF bricht auf dem
  Linux-Host mit `bad interpreter` ab.
- Abschnitt 9 gilt unveraendert. Das Skript erzeugt ein Image, es sagt nichts
  darueber aus, ob dieses Image auf dem Geraet startet.
