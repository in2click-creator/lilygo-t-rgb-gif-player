# Third-party components

The root MIT license covers new application code, documentation and tests.
It does not replace licenses on third-party source.

## Bundled source

**AnimatedGIF 2.2.3 source snapshot** — Larry Bank / BitBank Software.

- Upstream: https://github.com/bitbank2/AnimatedGIF
- Commit: `c2478eca7aa2f3b7a09bdc583e5a4a43d03ac0d9`
- License: Apache License 2.0.
- Location: `LilyGo_GifPlayer/src/AnimatedGIF/`.
- Full license: [LICENSE.txt](LilyGo_GifPlayer/src/AnimatedGIF/LICENSE.txt).
- Changes: [PROJECT_NOTES.md](LilyGo_GifPlayer/src/AnimatedGIF/PROJECT_NOTES.md).

The decoder was not written from scratch for this project. Its copyright and
license notices are retained. Local changes increase the width limit, rename
an implementation include for Arduino sketch packaging, and fix the palette
length check for very small GIF files.

## External build dependencies

These are installed separately rather than copied into the source repository:

- https://github.com/Xinyuan-LilyGO/LilyGo-T-RGB — 1.0.5
- https://github.com/lewisxhe/SensorLib — 0.2.3
- https://github.com/lvgl/lvgl — 8.3.11 (transitive LILYGO dependency)
- https://github.com/espressif/arduino-esp32 — 2.0.17, including its platform
  libraries and ESP-IDF dependencies.

Their own license terms continue to apply. A compiled firmware includes code
from these dependencies; the root MIT license is not a blanket relicense of
all binary contents.
