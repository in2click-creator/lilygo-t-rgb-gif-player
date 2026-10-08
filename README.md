# LILYGO T-RGB GIF Player

English | [Русский](README_RU.md)

A standalone microSD GIF player for the **LILYGO T-RGB 2.1 Full Circle**.
Swipe to browse animations; every image fills the circular screen with a
centred, aspect-preserving crop. Built with Arduino IDE.

**Status:** first hardware run reported working by the project owner.
The host rendering tests and ESP32-S3 compilation passed. Detailed hardware
checks are tracked in [VALIDATION.md](docs/VALIDATION.md).

## ALL-AI-POWERED development

The project owner defined the requirements, chose the interaction and tested the
physical device. OpenAI Codex generated the application code, tests and
documentation, integrated dependencies and prepared the repository. Existing
open-source libraries remain the work of their respective authors.

This describes the development process: **the firmware itself does not run an
AI model and requires no AI service or internet connection**.

## Features

- Finds `/gif` case-insensitively and sorts `.gif` files by name.
- Swipe left for next, right for previous; wraps around the file list.
- Loops the selected animation until you change it.
- Centre-crop scaling to fill the 480 × 480 display without aspect-ratio bars.
- Streams compressed data from microSD; no need to load the whole file.
- Handles local palettes, transparency, and disposal methods 2 and 3.
- On-screen errors with navigation to another file.
- Serial controls: `n` (next), `p` (previous), `r` (rescan).

## Hardware

| Part | Supported configuration |
|---|---|
| Board | LILYGO T-RGB **2.1 Full Circle** |
| MCU / storage | ESP32-S3, 16 MB flash, 8 MB OPI PSRAM |
| Display / touch | ST7701S, 480 × 480 / CST820 |
| Media | FAT32 microSD card |

Other T-RGB variants have **not** been validated. This is an independent
community project, not an official LILYGO firmware.

## Quick start — Arduino IDE

1. Download the repository ZIP and extract it, keeping its directory structure.
2. Install **esp32 by Espressif Systems 2.0.17** in Boards Manager. If necessary,
   add `https://espressif.github.io/arduino-esp32/package_esp32_index.json` to
   Additional Boards Manager URLs in Preferences.
3. In Library Manager install **LilyGo-T-RGB 1.0.5**, **SensorLib 0.2.3**, and
   **lvgl 8.3.11**. Explicitly check these versions if dependencies were
   installed automatically. AnimatedGIF is already bundled with this sketch.
4. Open `LilyGo_GifPlayer/LilyGo_GifPlayer.ino`. Do not use “Add .ZIP Library”:
   this repository is an application, not an Arduino library.
5. Apply the board settings below. New IDE windows may use different settings.
6. Create `gif` in the root of your microSD card and place GIF files directly
   inside it. For example, `/gif/001.gif`, `/gif/002.GIF`. Subfolders are not scanned.
7. Insert the card with the board powered off, connect USB, and upload.

### Board settings

| Arduino Tools setting | Value |
|---|---|
| Board | ESP32S3 Dev Module |
| Port | Your connected board's port |
| PSRAM | **OPI PSRAM** |
| Flash Size | 16MB (128Mb) |
| Flash Mode | QIO 80MHz |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| CPU Frequency | 240MHz (WiFi) |
| USB CDC On Boot | Enabled |
| USB Mode | Hardware CDC and JTAG |
| Upload Mode | UART0 / Hardware CDC |
| USB DFU On Boot | Disabled |
| USB Firmware MSC On Boot | Disabled |
| Erase All Flash Before Sketch Upload | Disabled |

Changing settings requires a fresh build and upload. Saving the `.ino` file
alone does not save these settings inside the sketch.

## Controls and display behaviour

Swipe horizontally at least 55 pixels. The image stays free of UI overlays
while playing. A swipe switches once per touch, after the current frame has
finished decoding. Serial Monitor runs at **115200 baud**; send `n`, `p`, or `r`
followed by Enter as a diagnostic alternative.

The shorter image dimension fills the screen. Excess content is cropped
symmetrically; the physical round display hides the corners. Scaling uses
nearest-neighbour sampling. Black content in a GIF stays black; transparent
areas are composited onto black. GIF loop-count metadata is intentionally
ignored: the selected item repeats until changed.

If touch directions are reversed on your setup, change `REVERSE_SWIPE` in
`LilyGo_GifPlayer/Player.cpp`. If the card or folder is missing, swipe to retry.
After changing card contents, restart or send `r` to rebuild the list.

## Limits and known issues

- Logical dimensions: **1–2048 pixels on each axis**. This is an acceptance
  limit, not a promise of smooth playback at the largest sizes.
- Up to **512 files** are indexed, then sorted. Additional files are ignored
  with a Serial warning. Sorting is lexical, case-insensitive for ASCII.
- No small megabyte cap; the decoder's signed file interface limits files to
  `INT32_MAX` bytes (approximately 2 GB).
- High-resolution or complex frames may slow playback and delay navigation.
  Zero frame delay becomes 100 ms; positive delays below 20 ms become 20 ms.
- Names are read from the filesystem unchanged, but the status font displays
  non-ASCII characters as `?`. Full names are available through Serial.
- A truncated GIF may play its decodable portion instead of reporting an error.
  This firmware is not a hardened parser for arbitrary hostile input.
- No Wi-Fi, audio, automatic card-change detection, or battery UI.

## Troubleshooting

| Symptom | Check |
|---|---|
| Black screen / `no mem for frame buffer` | Enable **OPI PSRAM**, rebuild and upload. |
| `NO SD CARD` | Card seating and FAT32; insert with power disconnected. |
| `FOLDER NOT FOUND` / `NO GIF FILES` | Files must be directly inside `/gif`. |
| `SIZE NOT SUPPORTED` | Resize the GIF to at most 2048 pixels per axis. |
| `GIF DECODE ERROR` | Try another file; include the Serial error in a bug report. |
| Swipes do not switch files | Try Serial `n` / `p`; report the touch model and board variant. |

## Development

Application code is in `LilyGo_GifPlayer`. The vendored decoder is in
`LilyGo_GifPlayer/src/AnimatedGIF`. [BUILD.md](docs/BUILD.md) contains reproducible
CLI commands and test instructions. CI checks host rendering and compiles the
Arduino firmware. Firmware binaries are build artifacts, not committed source.

See [CHANGELOG.md](CHANGELOG.md) for release history and
[CONTRIBUTING.md](CONTRIBUTING.md) for bug reports and contributions.

## Credits and licensing

Built on [LilyGo-T-RGB](https://github.com/Xinyuan-LilyGO/LilyGo-T-RGB),
[SensorLib](https://github.com/lewisxhe/SensorLib), and Larry Bank's
[AnimatedGIF](https://github.com/bitbank2/AnimatedGIF).

New application code and documentation are offered under the [MIT license](LICENSE).
The bundled AnimatedGIF source remains **Apache-2.0**; its original notices,
license and documented local modifications are preserved.
See [THIRD_PARTY.md](THIRD_PARTY.md).

Developed with assistance from OpenAI Codex. Initial requirements and hardware
validation were supplied by the project owner. No third-party GIF media is
redistributed; tests generate their own images.
