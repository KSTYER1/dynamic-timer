Dynamic Timer — OBS Filter Plugin v1.2.0
==========================================

Filter, der eine OBS-Textquelle in einen voll konfigurierbaren Timer
verwandelt. Filter-Enabled-State steuert den Timer (Augen-Icon), mehrere
Filter-Instanzen pro Quelle = Timer-Presets per Toggle.


Was ist neu in 1.2.0
--------------------

  * Zwei zusaetzliche Anzeigeformate:
      Nur Stunden  (z. B. "2")
      Nur Minuten  (z. B. "40")


Installation
------------

Empfohlen: Doppelklick auf INSTALL.bat, OBS-Pfad eingeben, fertig.

Manuell: BEIDE Top-Level-Ordner (obs-plugins\, data\) in den OBS-Hauptordner
kopieren. Ohne den data\-Teil zeigt OBS rohe Schluessel statt Beschriftungen.


Verzeichnisstruktur des Pakets
------------------------------

  dynamic-timer-1.2.0\
    INSTALL.bat
    README.txt
    obs-plugins\
      64bit\
        dynamic-timer.dll       <- Plugin-Binary (~27 KB)
        dynamic-timer.pdb       <- Debug-Symbole, optional
    data\
      obs-plugins\
        dynamic-timer\
          locale\
            en-US.ini
            de-DE.ini


Verwendung
----------

1. OBS starten, Text(GDI+)- oder Text(FreeType 2)-Quelle anlegen.
2. Auf der Textquelle: Rechtsklick -> Filter -> "+" -> "Dynamic Timer".
3. Modus auswaehlen. Mode-spezifische Felder erscheinen automatisch:
     Countdown        -> Stunden / Minuten / Sekunden eingeben
     Countup          -> Start-Offset eingeben
     Bis zu Uhrzeit   -> Stunde / Minute / Sekunde eingeben
     Bis zu Datum     -> Jahr / Monat / Tag + Stunde / Min / Sek
     Stream-Timer     -> keine Zeit-Eingabe, startet mit OBS-Stream
     Aufnahme-Timer   -> wie Stream, aber fuer Aufnahme
4. Anzeigeformat aus Liste waehlen (Default: HH:MM:SS).
5. Ggf. Aktion am Ende setzen:
     - Endtext (was bei 0 angezeigt wird, z. B. "Starting soon")
     - Nach Ablauf hochzaehlen (statt zu stoppen)
     - Beim Ende Szene wechseln (Zielszene auswaehlen)
6. Filter ueber das Augen-Icon aktivieren -> Timer startet.
7. Filter deaktivieren -> Timer pausiert.


Anzeigeformate
--------------

  MM:SS                 z. B.  05:30
  HH:MM:SS              z. B.  00:05:30   (Default)
  H:MM:SS               z. B.  0:05:30    (Stunden ohne fuehrende Null)
  MM:SS.HH              z. B.  05:30.42   (mit Hundertstel)
  Tage + Stunden        z. B.  1 T 03:14
  Nur Stunden           z. B.  2
  Nur Minuten           z. B.  40
  Nur Sekunden          z. B.  330


Hotkeys
-------

Pro Filter-Instanz (in OBS-Einstellungen -> Hotkeys):

  Dynamic Timer: Start/Stop (toggle filter)
    Toggled das Filter-Enabled-State (= Augen-Icon).

  Dynamic Timer: Reset
    Setzt den Timer auf den Anfang zurueck.


Multi-Instanz-Workflow
----------------------

Beispiel: drei Filter auf einer Textquelle "TimerText":

  Filter A: Countdown 0/5/0, Endtext "WERBUNG ZU ENDE"
  Filter B: Countup
  Filter C: Stream-Timer

  Filter A aktivieren -> 5-Min-Countdown laeuft.
  Filter A aus + B an -> Live-Counter laeuft.
  Filter A oder B aus + C an -> Stream-Dauer (sobald OBS streamt).

Genau ein Filter sollte gleichzeitig aktiv sein, damit nur ein Timer
in die Quelle schreibt.


Anforderungen
-------------

  - OBS Studio 30.x / 31.x / 32.x (Windows x64)
  - obs-frontend-api (von OBS mitgeliefert)


Versionshistorie
----------------

1.2.0 (2026-04-27)
  * Anzeigeformate erweitert: "Nur Stunden" und "Nur Minuten".

1.1.0 (2026-04-27)
  * UI-Aufraeumung:
    - Stunden / Minuten / Sekunden statt einer Gesamt-Sekunden-Zahl
    - Anzeigeformat-Presets statt Format-String-DSL
    - Property-Panel in vier Gruppen
  * Auto-Migration aus 1.0.0 (alte duration/offset/format-Settings).

1.0.0 (2026-04-27)
  * Erstrelease: Port von advanced-timer 10.lua
  * Filter-Enabled = Timer-Trigger (Multi-Instanz-faehig)
  * Alle 6 Modi des Originals, Format-DSL 1:1 portiert
  * Source-scoped Hotkeys: Toggle + Reset


Lizenz: GPLv2 oder neuer.
Source: https://github.com/kstyer/dynamic-timer
