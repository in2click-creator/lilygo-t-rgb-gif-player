# Changelog

## Unreleased — v1.0.2-perf diagnostics

- Optional Serial diagnostics: `d` toggles a summary every five seconds.
- Report observed submitted-frame rate, mean/max decode and output time,
  frames exceeding their processing budget, and mean/max GIF reopen time.
- Reset statistics when selecting another file; exclude status screens and
  metadata-only decoder calls. Skip reports when USB cannot accept the line.
- No playback optimisation yet; hardware measurements are pending.
- See `docs/PERFORMANCE_RU.md` for the measurement procedure and definitions.

## Unreleased — v1.0.1-test

- Reject invalid LZW minimum code sizes before accessing decoder tables.
- Do not add a 100 ms delay for trailing metadata after the final GIF frame.
- Add regression checks for all 249 invalid LZW size bytes, loop timing,
  and AddressSanitizer coverage of those cases.
- Provide generated device-check GIFs as a CI artifact and a hardware checklist.
- Basic hardware verification reported by the owner on 2026-10-09:
  both synthetic animations and personal GIFs play, swipes work, and the invalid
  LZW file shows an error with successful navigation to the next GIF.
  See `docs/VALIDATION.md` for the tested source and remaining checks.

## 1.0.0 — Initial release

- GIF playback from a case-insensitive `/gif` folder on microSD.
- Filename sorting and circular left/right swipe navigation.
- Centre-crop scaling for the 480×480 circular display.
- Screen-space RGB565 composition with transparency and disposal 2/3.
- On-screen error states and Serial navigation/rescan commands.
- Bundled and attributed AnimatedGIF decoder.
- English and Russian documentation, rendering tests and Arduino build CI.

First successful hardware run was reported on 2026-10-09 (Europe/Moscow).
