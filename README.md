# Dynamic Timer

Dynamic Timer is a third-party OBS Studio filter plugin that turns a text
source into a configurable timer.

The timer is controlled through the filter enabled state, so the eye icon acts
as start and pause. Multiple filter instances can be added to the same text
source to create timer presets that can be toggled when needed.

## Features

- Countdown timer.
- Countup timer.
- Countdown to a specific time of day.
- Countdown to a specific date and time.
- Streaming timer that follows the OBS streaming state.
- Recording timer that follows the OBS recording state.
- Multiple display presets, including `MM:SS`, `HH:MM:SS`, days plus hours,
  total hours, total minutes, and total seconds.
- Optional final text when a countdown reaches zero.
- Optional count-up after a countdown finishes.
- Optional scene switch when the timer finishes.
- Source-scoped hotkeys for start/stop and reset.
- Multi-instance workflow for timer presets on a single text source.

## Requirements

- OBS Studio 30.x, 31.x, or 32.x
- Windows x64
- A Text (GDI+) or Text (FreeType 2) source

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

Restart OBS after installation.

## Basic Usage

1. Create a Text (GDI+) or Text (FreeType 2) source.
2. Right-click the text source and open `Filters`.
3. Add `Dynamic Timer` as an effect filter.
4. Choose a mode and configure the mode-specific time settings.
5. Choose a display format.
6. Enable the filter with the eye icon to start the timer.
7. Disable the filter to pause the timer.

The filter writes the formatted timer text directly into the text source.

## Timer Modes

- `Countdown`: Counts down from a configured duration.
- `Countup`: Counts up from an optional start offset.
- `Specific time of day`: Counts down to a time today.
- `Specific date and time`: Counts down to an exact date and time.
- `Streaming timer`: Displays the current stream duration.
- `Recording timer`: Displays the current recording duration.

## Display Presets

- `MM:SS`, for example `05:30`
- `HH:MM:SS`, for example `00:05:30`
- `H:MM:SS`, for example `0:05:30`
- `MM:SS.HH`, for example `05:30.42`
- `Days + Hours`, for example `1 T 03:14`
- `Hours only`, for example `2`
- `Minutes only`, for example `40`
- `Seconds total`, for example `330`

## Hotkeys

Each filter instance exposes hotkeys in OBS settings:

- `Dynamic Timer: Start/Stop (toggle filter)`
- `Dynamic Timer: Reset`

Because the hotkeys are tied to the filter instance, you can create multiple
timer presets on the same text source and control them separately.

## Multi-Instance Presets

Example setup on one text source:

- Filter A: 5 minute countdown with final text
- Filter B: countup timer
- Filter C: stream timer

Enable one filter at a time to choose which timer preset currently controls the
text source.

## Version History

### 1.2.0

- Added `Hours only` and `Minutes only` display formats.

### 1.1.0

- Reworked the property UI into clearer groups.
- Replaced a total-seconds input with separate hours, minutes, and seconds
  fields.
- Replaced the format string workflow with display presets.
- Added automatic migration for 1.0.0 settings.

### 1.0.0

- Initial native plugin release.
- Ported the core workflow from an advanced timer Lua script.
- Added six timer modes.
- Added filter-enabled start/pause behavior.
- Added source-scoped toggle and reset hotkeys.

## License

Dynamic Timer is licensed under GPL-2.0-or-later.

## Disclaimer

Dynamic Timer is an unofficial third-party plugin and is not affiliated with or
endorsed by the OBS Project.

AI-assisted tools were used during development and release preparation. The
maintainer is responsible for reviewing, testing, and publishing the released
plugin.
