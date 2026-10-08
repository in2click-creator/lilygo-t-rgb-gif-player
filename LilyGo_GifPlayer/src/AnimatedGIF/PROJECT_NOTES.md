# Vendored AnimatedGIF

Upstream: https://github.com/bitbank2/AnimatedGIF

Source snapshot: commit `c2478eca7aa2f3b7a09bdc583e5a4a43d03ac0d9`.
Upstream library.properties reports version 2.2.3.
Author: Larry Bank / BitBank Software. License: Apache-2.0, included here.

Local changes for this sketch:

1. The MCU MAX_WIDTH limit is 2048 instead of 480. Player.cpp separately checks
   both logical dimensions before opening a file.
2. `gif.inl` is named `gif_impl.h` and its include updated so Arduino copies
   the implementation when building it as a sketch's bundled source.
3. The global-palette size check counts bytes already present in the first
   file read. Upstream's check compared only the second read length and could
   reject valid very small GIF files. Truncated palettes are still rejected.

The player uses RAW callbacks with its own RGB565 screen-space compositor.
It does not use COOKED or turbo modes. Palette changes, transparency, frame
rectangles, and disposal 2/3 are handled by GifCanvas.h.

4. Added local-modification notices and normalized trailing whitespace.
