# OBS Forum Submission Draft: Dynamic Timer

## Resource Title

Dynamic Timer

## Version

1.2.0

## Category

OBS Studio Plugins

## Tags

timer, countdown, countup, text source, filter, streaming timer, recording
timer, hotkeys

## Short Tagline

Turn an OBS text source into a configurable timer.

## Supported Bit Versions

64-bit

## Supported Platforms

Windows

## Minimum OBS Studio Version

30.0.0

## Source Code URL

TODO: add public GitHub repository URL

## Download URL

TODO: add GitHub Release URL or upload the ZIP directly

## Overview

Dynamic Timer is an unofficial third-party filter plugin for OBS Studio. It
turns a Text (GDI+) or Text (FreeType 2) source into a configurable timer.

The timer is controlled through the filter enabled state, so the filter eye icon
acts as start and pause. You can add multiple Dynamic Timer filters to the same
text source to create timer presets and toggle the one you need.

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

## Installation

Download the Windows x64 release archive and extract or copy its contents into
your OBS Studio installation directory.

The final layout should include:

```text
obs-plugins/64bit/dynamic-timer.dll
data/obs-plugins/dynamic-timer/locale/en-US.ini
data/obs-plugins/dynamic-timer/locale/de-DE.ini
```

The release archive also includes `INSTALL.bat`, which can copy the plugin into
a selected OBS directory.

Restart OBS after installation.

## Basic Usage

1. Create a Text (GDI+) or Text (FreeType 2) source.
2. Right-click the text source and open `Filters`.
3. Add `Dynamic Timer` as an effect filter.
4. Choose a mode and configure the time settings.
5. Choose a display format.
6. Enable the filter with the eye icon to start the timer.
7. Disable the filter to pause the timer.

## Timer Modes

- `Countdown`
- `Countup`
- `Specific time of day`
- `Specific date and time`
- `Streaming timer`
- `Recording timer`

## Display Presets

- `MM:SS`, for example `05:30`
- `HH:MM:SS`, for example `00:05:30`
- `H:MM:SS`, for example `0:05:30`
- `MM:SS.HH`, for example `05:30.42`
- `Days + Hours`, for example `1 T 03:14`
- `Hours only`, for example `2`
- `Minutes only`, for example `40`
- `Seconds total`, for example `330`

## What's New in 1.2.0

- Added `Hours only` and `Minutes only` display formats.

## Support / Bugs

Please report issues in the resource discussion thread or in the GitHub issue
tracker once the repository is published.

## License

GPL-2.0-or-later.

## Disclaimer

Dynamic Timer is an unofficial third-party plugin and is not affiliated with or
endorsed by the OBS Project.

AI-assisted tools were used during development and release preparation. The
maintainer is responsible for reviewing, testing, and publishing the released
plugin.

## Pre-Submit Checklist

- [ ] Public GitHub repository exists.
- [ ] README is visible on GitHub.
- [ ] GPL license is visible on GitHub.
- [ ] Source Code URL field points to the repository.
- [ ] Release ZIP is attached to GitHub Releases or uploaded to the forum.
- [ ] At least one screenshot/GIF is added to the resource description.
- [ ] Description is in English.
- [ ] No OBS logo is used as resource icon or marketing artwork.
