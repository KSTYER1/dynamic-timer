# Dynamic Timer

Dynamic Timer is a third-party OBS Studio filter plugin that turns an OBS text
source into a configurable timer.

The filter enabled state controls whether the timer is active, which makes it
useful for multi-instance preset workflows on the same source.

## Features

- Countdown mode
- Count-up mode
- Count down to a specific time
- Count down to a specific date
- Stream timer mode
- Recording timer mode
- End text support
- Optional continue-counting-after-zero behavior
- Optional scene switch on completion
- Multiple display formats including hours-only and minutes-only output
- Per-filter hotkeys for toggle and reset

## Requirements

- OBS Studio 30.x, 31.x, or 32.x
- Windows x64 for the packaged release
- `obs-frontend-api`, provided by OBS Studio

## Installation

### Windows

Download the release archive and extract or copy its contents into your OBS
Studio installation directory.

The final layout should include:

```text
obs-plugins/64bit/dynamic-timer.dll
data/obs-plugins/dynamic-timer/locale/en-US.ini
data/obs-plugins/dynamic-timer/locale/de-DE.ini
```

The packaged release also includes `INSTALL.bat`, which can copy the plugin
files into a selected OBS directory.

Restart OBS after installation. The filter appears in the filter menu for
`Text (GDI+)` and `Text (FreeType 2)` sources.

## Basic Usage

1. Create or select an OBS text source.
2. Open the source filters.
3. Add `Dynamic Timer`.
4. Choose the timer mode and fill in the mode-specific settings.
5. Select a display format.
6. Enable the filter to start the timer.
7. Disable the filter to pause it.

For multi-preset workflows, you can place multiple Dynamic Timer filters on
the same source and enable only the one you want to run.

## Display Formats

- `MM:SS`
- `HH:MM:SS`
- `H:MM:SS`
- `MM:SS.HH`
- `Days + Hours`
- `Hours Only`
- `Minutes Only`
- `Seconds Only`

## Version History

### 1.2.0

- Added `Hours Only` and `Minutes Only` display formats.

### 1.1.0

- Reworked the UI around grouped properties.
- Replaced single total-second fields with hour, minute, and second fields.
- Added migration from the earlier 1.0.0 settings layout.

### 1.0.0

- Initial release.
- Ported the original `advanced-timer 10.lua` behavior.
- Added source-scoped hotkeys for toggle and reset.

## License

Dynamic Timer is licensed under GPL-2.0-or-later.

## Disclaimer

Dynamic Timer is an unofficial third-party plugin and is not affiliated with
or endorsed by the OBS Project.
