# Validation status

## v1.0.1-test: host regressions

- The existing 15-file / 51-frame Pillow comparison passes after the fixes.
- All 249 illegal LZW minimum-code-size bytes are rejected cleanly in the
  host runner, including under AddressSanitizer.
- A four-frame GIF with 50/100/150/200 ms delays and the same GIF with a trailing
  comment produce identical frames and 500 ms of scheduled delay per loop.
  The metadata-only EOF call schedules no extra delay.
- The shared timing helper preserves the 100 ms default for displayed frames
  with zero delay and the 20 ms minimum for positive short delays.
- These are host checks, not FPS measurements or hardware validation.
- Hosted rendering, AddressSanitizer and ESP32-S3 firmware jobs passed in
  [Actions run 37854773258](https://github.com/in2click-creator/lilygo-t-rgb-gif-player/actions/runs/37854773258).

## v1.0.1-test: owner hardware report, 2026-10-09

The owner was supplied source commit
`614d8d4cfb3fb463bfcdeab54eafbfa1b1c6215d` for the T-RGB 2.1 Full Circle.
At 01:52 Europe/Moscow the owner reported:

- Both synthetic animations play normally; colours cycle.
- Personal GIFs play normally, with left/right swipe navigation.
- `003-invalid-lzw.gif` shows `GIF DECODE ERROR / DECODER CODE 1`, at 3 / 24.
- The next swipe opens a working personal GIF; the error does not trap the UI.

**Basic hardware check passed by owner report.** The supplied Serial excerpt
shows `001-normal.gif` opening at 96 x 96, 789 bytes. The startup version line
was not captured. Exact timing, Serial navigation, first/last wraparound and a
long-running stability test were not separately confirmed. The extra-delay fix
is verified quantitatively by the host regression, not a device timing capture.
See the [hardware checklist](HARDWARE_CHECK_RU.md) and
[PR report](https://github.com/in2click-creator/lilygo-t-rgb-gif-player/pull/1#issuecomment-6070643508).

## Completed before publication

- Arduino compilation passed for ESP32-S3 with core 2.0.17, OPI PSRAM and 16 MB
  flash, using LilyGo-T-RGB 1.0.5, SensorLib 0.2.3 and lvgl 8.3.11.
- The application image used 404,073 bytes, with 59,560 bytes of static internal
  RAM in the initial local build. Toolchain differences may affect exact sizes.
- 15 generated GIF files / 51 frames matched a Pillow reference after RGB565
  conversion and centre-crop scaling. Tests cover landscape, portrait, square,
  transparency, disposal 1/2/3, and sizes from 1×1 to 1600×900.
- The project owner uploaded the application to a physical T-RGB 2.1 Full Circle
  and reported it working on 2026-10-09, Europe/Moscow.

## Still to document on hardware

- Explicit left=next/right=previous mapping, wraparound, colour and orientation checks.
- Cold-start checks with multiple cards and files.
- Missing card, empty folder, other malformed files and card read-error scenarios.
- Measured FPS / frame times with representative media.

A successful initial run is not an exhaustive compatibility guarantee.
The initial main commit passed both hosted CI checks. New commits must pass
their own firmware and host-test jobs; see the Actions run for the exact commit.
