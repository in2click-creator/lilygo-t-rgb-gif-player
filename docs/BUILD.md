# Build and test

Arduino IDE is the recommended path for first-time users; see the root README.
The following commands are for development using Arduino CLI and a Linux shell.

## Firmware

```sh
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32@2.0.17
arduino-cli lib update-index
arduino-cli lib install --no-deps 'LilyGo-T-RGB@1.0.5' 'SensorLib@0.2.3' 'lvgl@8.3.11'
python3 -m pip install pyserial reedsolo bitstring ecdsa intelhex
arduino-cli compile \
  --fqbn 'esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=16M,FlashMode=qio,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi' \
  --output-dir build/firmware LilyGo_GifPlayer
```

Arduino CLI users who already have a configuration should update it rather than
replace it. The Python packages are dependencies of the older esptool included
with core 2.0.17; use a virtual environment if required by your OS.

## Host rendering tests

Requires g++, Python 3.12 and the packages below. Run from the repository root:

```sh
python3 -m pip install -r tests/requirements.txt
g++ -std=c++11 -D__LINUX__ -O2 \
  tests/decoder_test.cpp LilyGo_GifPlayer/src/AnimatedGIF/AnimatedGIF.cpp \
  -o tests/decoder_test
python3 tests/test_gifs.py
g++ -std=c++11 tests/timing_test.cpp -o /tmp/timing_test
/tmp/timing_test
python3 tests/test_regressions.py
g++ -std=c++11 -D__LINUX__ -O1 -g -fsanitize=address -fno-omit-frame-pointer \
  tests/decoder_test.cpp LilyGo_GifPlayer/src/AnimatedGIF/AnimatedGIF.cpp \
  -o /tmp/decoder_sanitized
DECODER_TEST=/tmp/decoder_sanitized python3 tests/test_regressions.py
```

Tests generate their own GIFs, decode through file read/seek callbacks, and
compare every output pixel against Pillow after compositing, RGB565 conversion
and centre-crop scaling. The generated fixtures and raw frames are ignored by Git.
The test suite does not emulate SD electrical behaviour or touchscreen hardware.
Regression tests also compare normal/trailing-comment loop timing, reject all
249 illegal LZW minimum-code-size bytes, and generate the three hardware-check
GIFs in `tests/fixtures/device/gif`. The shared timing helper is exercised on the
host; the full Arduino event loop and hardware drivers are not emulated.
CI publishes the files as `lilygo-device-check`; follow
[`HARDWARE_CHECK_RU.md`](HARDWARE_CHECK_RU.md) on the actual board.

## Release

The initial-release CI job creates tag `v1.0.0` and a GitHub release after both
checks pass on main, using `docs/RELEASE_v1.0.0.md`. If that release already exists,
it is left unchanged. GitHub automatically provides
source archives. Keep licenses with any separately packaged source distribution.
Binaries require their precise board settings and flashing offsets; the first
release is source-first and does not ask users to guess these offsets.
