# Changelog - OpenWRT_Update

Format: SemVer + ISO-Datum. Sektionen: Hinzugefuegt / Geaendert / Behoben / Entfernt / Verifiziert.

## [0.2.3] - 2026-09-15
### Entfernt
- Gitlinks `openwrt-RS353_1` und `openwrt-RS353_2` (Modus `160000`) aus dem
  Index. Es gab kein `.gitmodules`, also weder Submodul noch Inhalt; auf GitHub
  ergab das zwei Ordner, deren Klick ins Leere fuehrt. `git rm --cached`
  scheitert an solchen Eintraegen (`pathspec did not match any files`), entfernt
  wurde mit `git update-index --force-remove`. Arbeitsverzeichnisse unberuehrt.

### Hinzugefuegt
- `README.md`: Abschnitt "Herkunft" mit Repo-URL, Branch, Commit-SHA und Datum
  beider Forks, Klon-Kommando und Hinweis, dass auf `master` von
  `openwrt-RS353_2` keine RS353-Dateien liegen.
- `.gitignore`: `openwrt-RS353_1/`, `openwrt-RS353_2/` gesperrt, mit
  Begruendung (rund 500 MB Fremdcode ohne Nutzen fuer den Build).

### Geaendert
- `README.md`: Zweck und Struktur-Tabelle nennen die Forks als lokale Referenz
  ausserhalb des Repos; Abschnitt "Git-Stand" erklaert die entfernten Gitlinks;
  `port/`-Zeile auf den tatsaechlichen Inhalt gebracht.
- `MVC.md`: Fremdcode-Bullet vermerkt "nur lokal, seit 2026-09-15 nicht im Git".
- `ERKLAERUNG.md`: Entscheidung Gitlink-Entfernung gegen Submodul und gegen
  Einchecken des Fremdcodes, mit Begruendung.

### Verifiziert
- `git ls-files -s | grep 160000` liefert keine Zeile mehr.
- `git status --short`: `D openwrt-RS353_1`, `D openwrt-RS353_2` gestaged,
  23 Dateien getrackt.
- `openwrt-RS353_1/.git` und `openwrt-RS353_2/.git` liegen unveraendert auf
  Platte; `git check-ignore -v` weist beide Ordner den `.gitignore`-Zeilen zu.
- NICHT erledigt: Commit und Push. Der Remote-Stand `af2805d` enthaelt die
  kaputten Gitlinks weiterhin, bis gepusht wird.

## [0.2.2] - 2026-09-15
### Hinzugefuegt
- `.gitattributes`: `* text=auto eol=lf` plus explizite Regeln fuer `*.sh`,
  `*.patch`, `*.diff`, `*.dts`, `*.c`, `*.h`, `Makefile`, `Dockerfile`;
  `*.cev`, `*.bin`, `*.img`, `*.gz` als `binary`. Grund: `core.autocrlf=true`
  ist auf dem Windows-Host gesetzt, ein Windows-Klon bekaeme sonst CRLF. Ein
  Skript mit CRLF bricht auf dem Linux-Host mit `bad interpreter` ab, ein Patch
  mit CRLF wird von `git apply` verworfen.
- `.gitignore`: Ausnahmen `!port/apply.sh`, `!port/phase4.sh`,
  `!port/tree/target/linux/lantiq/base-files/etc/uci-defaults/09_fix_crc.sh`.

### Geaendert
- `README.md`: Abschnitt "Bekannte Luecke im Git-Stand" ersetzt durch
  "Git-Stand" mit Tabelle der vier ausgenommenen Skripte und Hinweis auf die
  LF-Erzwingung.
- `BUILD_HOWTO.md` 13.3: Grenze "Port-Skripte fehlen im Repo" entfaellt, statt
  dessen Hinweis auf LF-Zwang und alte CRLF-Klone.

### Behoben
- Letzte Zeile in `.gitignore` hatte CRLF, jetzt durchgaengig LF.

### Verifiziert
- `git check-ignore -q` meldet fuer alle vier Skripte "trackbar"; `git add -n`
  listet `.gitattributes`, `.gitignore`, `port/apply.sh`, `port/phase4.sh`,
  `port/tree/.../09_fix_crc.sh`.
- `git check-attr text eol` liefert `text: set / eol: lf` fuer `*.sh` und
  `*.patch`, `text: auto / eol: lf` fuer `*.md`.
- `git ls-files --eol`: `i/lf w/lf` fuer alle geprueften Dateien, keine
  Renormalisierung noetig.
- `tr -cd CR | wc -c` auf `.gitattributes` und `.gitignore`: 0 Bytes.
- NICHT verifiziert: Verhalten eines frischen Windows-Klons. Auf diesem Host
  wurde kein zweiter Klon gezogen.
- NICHT behoben: die beiden vor Sessionstart vorhandenen Commits. Lokaler
  Reflog kennt nur `bd3affc`, `git fsck --lost-found` ist leer, `origin/main`
  steht ebenfalls auf `bd3affc`. Nicht wiederherstellbar.

## [0.2.1] - 2026-09-15
### Hinzugefuegt
- `BUILD_HOWTO.md`: verlinktes Inhaltsverzeichnis (55 Eintraege, Abschnitte 0
  bis 13 samt F1-F12), eingefasst in `<!-- TOC -->` / `<!-- /TOC -->`.

### Geaendert
- `README.md`: Struktur um `port/`, `docker/`, `build_rs353_linux.sh`, `out/`
  ergaenzt; Setup nach Linux- und Windows-Weg getrennt; Aufruf auf das
  Build-Skript umgestellt; Artefaktname `*-boss-image.cev`; Hinweis auf den
  seriellen Erstflash; Git-Luecke bei `port/*.sh` benannt.
- `INFRA.md`: Artefakt von `*-sysupgrade.bin` auf
  `*-bintec_rs353-boss-image.cev` korrigiert; Erstflash ueber Bootmonitor statt
  Web-UI/sysupgrade; Build-Host-Tabelle Linux/Windows; Arbeitsordner, Named
  Volume, Groessengrenze 31232k, serielle Parameter ergaenzt.
- `MVC.md`: Eigencode eingeordnet (`build_rs353_linux.sh`, `port/apply.sh`,
  `port/phase4.sh` als Service-Schicht; `port/patches`, `port/tree` als Daten);
  Root-Ablage des Einstiegsskripts als bewusste Abweichung begruendet.
- `ERKLAERUNG.md`: Entscheidungen zu Docker als Windows-Kruecke, Skript statt
  Kommandoliste, Endmarker statt Pipe, Zeitstempel-Fund des Images, `-j4`,
  unveraendertes `IMAGE_SIZE`; offener Punkt Geraete-Reife.

### Verifiziert
- TOC-Anker aus den Ueberschriften erzeugt (GitHub-Schema), Codebloecke
  uebersprungen, `<name>` in F4 als `&lt;name&gt;` maskiert.
- NICHT verifiziert: gerenderte Sprungziele. Auf diesem Host wurde kein
  Markdown-Renderer ausgefuehrt.

## [0.2.0] - 2026-09-15
### Hinzugefuegt
- `build_rs353_linux.sh`: Ein-Kommando-Build fuer Laien auf einem Linux-Host,
  ohne Docker. Zehn Schritte mit je eigener Erfolgspruefung: Nicht-root-Test,
  Paketinstallation nach `docker/Dockerfile`, Test auf Gross-/Kleinschreibung
  und 40 GB Platz, Klon `openwrt-24.10`, Feeds, `port/apply.sh` plus fuenf
  Pruefmarken, Profil mit `.config.bak`, `make -j4` ohne Pipe mit Endmarker
  `RS353_BUILD_OK`/`RS353_BUILD_FAIL`, Stamp-Frischepruefung und 31232k-Limit,
  Kopie nach `out/`. Optionen `--workdir`, `--jobs`, `--skip-deps`.

### Geaendert
- `BUILD_HOWTO.md`: neuer Abschnitt 13 (Linux-Host ohne Docker) mit Aufruf,
  Optionen, Zuordnung der Skript-Schritte zu den Docker-Abschnitten und
  Grenzen. Abschnitt 0 nennt Docker nun ausdruecklich als Windows-Kruecke.
- `.gitignore`: `!build_rs353_linux.sh` in die Ausnahmeliste der `*.sh`-Regel.

### Verifiziert
- `bash -n build_rs353_linux.sh` -> SYNTAX_OK. Datei ASCII, LF, keine CR.
- `git add -n build_rs353_linux.sh` -> `add 'build_rs353_linux.sh'`, die
  Whitelist greift.
- NICHT verifiziert: ein echter Durchlauf. Der Entwicklungshost ist Windows,
  ein Linux-Build war nicht ausfuehrbar. `shellcheck` ist hier nicht
  installiert. Der erste Lauf auf einem Linux-Rechner steht aus.

## [0.1.0] - 2026-09-14
### Hinzugefuegt
- Initiales Scaffold (project: OpenWRT_Update) via std-workflow.
- Pflicht-Doku befuellt: README.md, INFRA.md, MVC.md, ERKLAERUNG.md.
- PLAN.md: Ausgangslage beider RS353-Forks, Portierungsschritte, offene Punkte.
- .gitignore mit Pflichteintraegen.

### Behoben
- BOM aus .gitignore entfernt; README-Aussage zum RS353-Support je Tree korrigiert;
  z_INFOS/ und restore.py in README-Struktur ergaenzt.

### Verifiziert
- Restore-Baseline angelegt (7693 Dateien, 2026-09-14_10-37-23).
- Scaffold idempotent durchgelaufen, 13 Objekte erzeugt.
- Fork-Stand aus Git-Historie gelesen (Xernium openwrt-22.03-old 2023-02-15;
  armSeb master 2022-10-26), RS353-Dateien in Tree _1 lokalisiert.
- Doku-Review durch Subagent: Befunde behoben, Git-Fakten und Dateipfade bestaetigt.
