# v1.0.0 — First working release

A microSD GIF player for LILYGO T-RGB 2.1 Full Circle, with swipe navigation
and centred crop-to-fill rendering.

Open `LilyGo_GifPlayer/LilyGo_GifPlayer.ino` in Arduino IDE. Use ESP32 core
2.0.17 and **OPI PSRAM**; follow the board settings in the README.

Includes GIF file discovery, lexical sorting, looped playback, transparency,
error screens and Serial fallback controls. GIF dimensions up to 2048×2048
are accepted; performance depends on the file. Up to 512 files are indexed.

Initial hardware run confirmed by the project owner. Compilation and 51 host
reference frames passed. Full hardware error-path and performance validation
remain open. Other T-RGB models are not validated.

See CHANGELOG.md, docs/VALIDATION.md and THIRD_PARTY.md for details.
