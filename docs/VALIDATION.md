# Validation status

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

- Explicit swipe-direction, wraparound, colour and orientation checks.
- Cold-start checks with multiple cards and files.
- Missing card, empty folder, malformed file and card read-error scenarios.
- Measured FPS / frame times with representative media.

A successful initial run is not an exhaustive compatibility guarantee.
The CI workflow is prepared; its first hosted run can happen only after push.
