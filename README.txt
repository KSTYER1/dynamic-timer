Dynamic Timer - OBS Filter Plugin v1.2.0
========================================

Dynamic Timer turns an OBS text source into a configurable timer.

The filter enabled state acts as the timer trigger, which makes it useful for
multi-instance timer presets on the same source.


What's New in 1.2.0
-------------------

  * Added `Hours Only` and `Minutes Only` display formats


Package Contents
----------------

  dynamic-timer-1.2.0\
    INSTALL.bat
    README.txt
    obs-plugins\
      64bit\
        dynamic-timer.dll
        dynamic-timer.pdb
    data\
      obs-plugins\
        dynamic-timer\
          locale\
            en-US.ini
            de-DE.ini


Installation
------------

Recommended: double-click `INSTALL.bat`, choose your OBS folder, and let the
script copy the files for you.

Manual installation: copy BOTH top-level folders, `obs-plugins\` and `data\`,
into your OBS Studio installation directory.


Usage
-----

Add `Dynamic Timer` to an OBS `Text (GDI+)` or `Text (FreeType 2)` source.

Supported modes:

  - Countdown
  - Count-up
  - To specific time
  - To specific date
  - Stream timer
  - Recording timer


Requirements
------------

  - OBS Studio 30.x / 31.x / 32.x (Windows x64)
  - obs-frontend-api (provided by OBS)


License
-------

GPLv2 or later.
