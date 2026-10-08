#include "Player.h"
#include <Arduino.h>
#include <esp_arduino_version.h>
#include <esp_heap_caps.h>
#include <LilyGo_RGBPanel.h>
#include <SD_MMC.h>
#include <algorithm>
#include <limits.h>
#include "GifCanvas.h"
#include "PlaybackTiming.h"
#include "TinyText.h"

#if ESP_ARDUINO_VERSION_MAJOR != 2
#error "Use esp32 by Espressif Systems 2.0.17 for this project."
#endif
#ifndef BOARD_HAS_PSRAM
#error "Tools > PSRAM > OPI PSRAM is required."
#endif

namespace {
constexpr int MAX_GIFS = 512;
constexpr int MAX_DIMENSION = 2048;
constexpr int SWIPE_PIXELS = 55;
constexpr bool REVERSE_SWIPE = false; // Set true if left/right feel reversed.

LilyGo_RGBPanel panel;
AnimatedGIF decoder;
GifCanvas canvas;
File gifFile;
String paths[MAX_GIFS];
String folder;
int count = 0, selected = 0;
bool panelReady = false, sdReady = false, playing = false;
bool decoderOpen = false, ioFault = false, restartPending = false;
bool requestRescan = false;
int pendingStep = 0;
uint32_t nextFrameAt = 0, lastInputAt = 0;
uint32_t framesPlayed = 0;

String basenameOf(const String &s) {
    return s.substring(s.lastIndexOf('/') + 1);
}

void present() {
    // LilyGo's API takes the rectangle's ending coordinates in its last
    // two coordinate arguments. At origin (0,0), these are both 480.
    panel.pushColors(0, 0, 480, 480, canvas.pixels);
}

void message(const char *title, const char *detail, bool navigation = true) {
    Serial.printf("[%s] %s\n", title, detail);
    if (!panelReady || !canvas.pixels) return;
    std::fill_n(canvas.pixels, GifCanvas::PIXELS, (uint16_t)0x0841);
    TinyText::centred(canvas.pixels, 130, "GIF PLAYER", 0x07ff, 3);
    TinyText::centred(canvas.pixels, 198, title, 0xffe0, 2);
    TinyText::centred(canvas.pixels, 235, detail);
    if (count > 0) {
        TinyText::centred(canvas.pixels, 273, basenameOf(paths[selected]).c_str());
        char pos[32]; snprintf(pos, sizeof(pos), "%d / %d", selected+1, count);
        TinyText::centred(canvas.pixels, 306, pos);
    }
    if (navigation) TinyText::centred(canvas.pixels, 347,
                                    count ? "< SWIPE TO CHANGE >" : "SWIPE TO RETRY");
    present();
}

void closeGif() {
    playing = false;
    if (decoderOpen) decoder.close();
    decoderOpen = false;
    if (gifFile) gifFile.close();
    restartPending = false;
}

void failure(const char *title, const char *detail) {
    closeGif();
    message(title, detail);
}

void pollInput() {
    uint32_t now = millis();
    if ((uint32_t)(now-lastInputAt) < 10) return;
    lastInputAt = now;
    static bool held = false, fired = false;
    static int16_t startX = 0, startY = 0;
    static uint32_t lastTouchAt = 0;
    int16_t x = 0, y = 0;
    bool touched = panelReady && panel.getPoint(&x, &y, 1);
    if (touched) {
        if (!held || (uint32_t)(now-lastTouchAt) > 120) {
            held = true; fired = false; startX = x; startY = y;
        }
        lastTouchAt = now;
        int dx = x-startX, dy = y-startY;
        if (!fired && abs(dx) >= SWIPE_PIXELS && abs(dx)*10 > abs(dy)*13) {
            int step = dx < 0 ? 1 : -1;
            pendingStep = REVERSE_SWIPE ? -step : step;
            fired = true;
        }
    } else if (held && (uint32_t)(now-lastTouchAt) > 80) {
        held = false; fired = false;
    }
    while (Serial.available()) {
        char c = Serial.read();
        if (c == 'n' || c == 'N') pendingStep = 1;
        if (c == 'p' || c == 'P') pendingStep = -1;
        if (c == 'r' || c == 'R') requestRescan = true;
    }
}

void *gifOpen(const char *name, int32_t *size) {
    gifFile = SD_MMC.open(name, FILE_READ);
    if (!gifFile || gifFile.isDirectory() || gifFile.size() > INT32_MAX) {
        if (gifFile) gifFile.close();
        return nullptr;
    }
    *size = (int32_t)gifFile.size();
    return &gifFile;
}
void gifClose(void *handle) {
    if (handle) static_cast<File *>(handle)->close();
}
int32_t gifRead(GIFFILE *f, uint8_t *buffer, int32_t requested) {
    pollInput();
    if (!f || !f->fHandle || requested <= 0) return 0;
    File *file = static_cast<File *>(f->fHandle);
    int32_t remaining = f->iSize-f->iPos;
    if (remaining <= 0) return 0;
    int32_t amount = requested < remaining ? requested : remaining;
    int32_t got = (int32_t)file->read(buffer, (size_t)amount);
    if (got < 0) got = 0;
    if (got != amount) ioFault = true;
    f->iPos = (int32_t)file->position();
    return got;
}
int32_t gifSeek(GIFFILE *f, int32_t position) {
    pollInput();
    if (!f || !f->fHandle || position < 0 || position > f->iSize) {
        ioFault = true;
        return -1;
    }
    File *file = static_cast<File *>(f->fHandle);
    if (!file->seek((uint32_t)position)) { ioFault = true; return -1; }
    f->iPos = (int32_t)file->position();
    return f->iPos;
}
void gifDraw(GIFDRAW *d) {
    pollInput(); // Also runs during decoding, not just between frames.
    canvas.draw(*d);
}

bool inspectGif(const String &path, int &w, int &h, uint16_t &background, bool logFile) {
    File f = SD_MMC.open(path, FILE_READ);
    if (!f || f.isDirectory()) {
        message("FILE ERROR", "CANNOT OPEN GIF");
        return false;
    }
    if (f.size() > INT32_MAX) {
        f.close(); message("FILE TOO LARGE", "LIMIT IS 2 GB"); return false;
    }
    uint8_t header[13];
    if (f.read(header, sizeof(header)) != sizeof(header) ||
        (memcmp(header, "GIF87a", 6) && memcmp(header, "GIF89a", 6))) {
        f.close(); message("BAD FILE", "NOT A VALID GIF"); return false;
    }
    w = header[6] | (header[7] << 8);
    h = header[8] | (header[9] << 8);
    if (w < 1 || h < 1 || w > MAX_DIMENSION || h > MAX_DIMENSION) {
        char note[31]; snprintf(note, sizeof(note), "%d X %d / MAX 2048", w, h);
        f.close(); message("SIZE NOT SUPPORTED", note); return false;
    }
    background = 0;
    if (header[10] & 0x80) {
        unsigned paletteSize = 1u << ((header[10] & 7) + 1);
        if (f.size() < 13 + paletteSize*3u || header[11] >= paletteSize) {
            f.close(); message("BAD FILE", "INVALID COLOR TABLE"); return false;
        }
        uint8_t rgb[3];
        if (!f.seek(13 + header[11]*3u) || f.read(rgb, 3) != 3) {
            f.close(); message("READ ERROR", "CANNOT READ PALETTE"); return false;
        }
        background = ((rgb[0] & 0xf8) << 8) | ((rgb[1] & 0xfc) << 3) | (rgb[2] >> 3);
    }
    if (logFile) Serial.printf("GIF: %s | %d x %d | %lu bytes\n", path.c_str(), w, h,
                              (unsigned long)f.size());
    f.close();
    return true;
}

void openSelected(bool showLoading) {
    closeGif();
    if (!count) return;
    if (showLoading) message("LOADING", "PLEASE WAIT", false);
    int w = 0, h = 0; uint16_t background = 0;
    if (!inspectGif(paths[selected], w, h, background, showLoading)) return;
    canvas.configure(w, h, background);
    ioFault = false;
    decoder.begin(GIF_PALETTE_RGB565_LE);
    if (!decoder.open(paths[selected].c_str(), gifOpen, gifClose, gifRead, gifSeek, gifDraw)) {
        int err = decoder.getLastError();
        if (gifFile) gifFile.close();
        char note[31]; snprintf(note, sizeof(note), "DECODER CODE %d", err);
        message("CANNOT OPEN GIF", note);
        return;
    }
    decoderOpen = true;
    playing = true;
    framesPlayed = 0;
    nextFrameAt = millis();
}

void scanCard() {
    closeGif();
    for (int i = 0; i < count; ++i) paths[i] = String();
    count = 0; selected = 0; folder = String();
    message("READING CARD", "LOOKING FOR /GIF", false);
    if (sdReady) panel.uninstallSD();
    sdReady = panel.installSD();
    if (!sdReady) {
        // Also clean up a partially mounted filesystem before the next retry.
        panel.uninstallSD();
        message("NO SD CARD", "CHECK CARD / FAT32");
        return;
    }
    File root = SD_MMC.open("/");
    if (!root) { message("SD ERROR", "CANNOT READ ROOT"); return; }
    File entry = root.openNextFile();
    while (entry) {
        String name = basenameOf(String(entry.name()));
        if (entry.isDirectory() && name.equalsIgnoreCase("gif")) {
            folder = String("/") + name;
            entry.close(); break;
        }
        entry.close(); entry = root.openNextFile();
        pollInput();
    }
    root.close();
    if (!folder.length()) { message("FOLDER NOT FOUND", "CREATE /GIF ON SD"); return; }
    File dir = SD_MMC.open(folder);
    if (!dir || !dir.isDirectory()) { message("FOLDER ERROR", "CANNOT OPEN /GIF"); return; }
    bool truncated = false;
    entry = dir.openNextFile();
    while (entry) {
        String name = basenameOf(String(entry.name()));
        if (!entry.isDirectory() && name.length() > 4 &&
            name.substring(name.length()-4).equalsIgnoreCase(".gif")) {
            if (count == MAX_GIFS) { truncated = true; entry.close(); break; }
            paths[count++] = folder + "/" + name;
        }
        entry.close(); entry = dir.openNextFile();
        pollInput();
    }
    dir.close();
    if (!count) { message("NO GIF FILES", "PUT GIF FILES IN /GIF"); return; }
    std::sort(paths, paths + count, [](const String &a, const String &b) {
        int c = strcasecmp(a.c_str(), b.c_str());
        return c ? c < 0 : strcmp(a.c_str(), b.c_str()) < 0;
    });
    Serial.printf("Found %d GIF files in %s%s\n", count, folder.c_str(),
                  truncated ? " (only first 512 indexed)" : "");
    for (int i = 0; i < count; ++i) Serial.printf("%d: %s\n", i+1, paths[i].c_str());
    pendingStep = 0; requestRescan = false;
    openSelected(true);
}
}

void playerSetup() {
    Serial.begin(115200);
    Serial.println("\nLILYGO GIF PLAYER v1.0.1-test");
    Serial.printf("PSRAM: %lu bytes, free: %lu\n",
                  (unsigned long)ESP.getPsramSize(), (unsigned long)ESP.getFreePsram());
    if (!psramFound()) {
        Serial.println("ERROR: Enable OPI PSRAM in Tools and upload again.");
        return;
    }
    size_t bytes = GifCanvas::PIXELS * sizeof(uint16_t);
    canvas.pixels = (uint16_t *)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    canvas.saved = (uint16_t *)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    panelReady = panel.begin(LILYGO_T_RGB_2_1_INCHES_FULL_CIRCLE);
    if (!panelReady) { Serial.println("ERROR: Display initialization failed."); return; }
    panel.setBrightness(16);
    if (!canvas.pixels || !canvas.saved) {
        message("MEMORY ERROR", "CHECK OPI PSRAM", false);
        panelReady = false;
        return;
    }
    Serial.printf("Touch: %s\n", panel.getTouchModelName());
    Serial.println("Swipe left: next. Swipe right: previous. Serial: n / p / r (rescan).");
    scanCard();
}

void playerLoop() {
    if (!panelReady) { delay(20); return; }
    pollInput();
    if (requestRescan || (pendingStep && !count)) {
        pendingStep = 0; requestRescan = false; scanCard();
    } else if (pendingStep) {
        int step = pendingStep; pendingStep = 0;
        selected = (selected + step + count) % count;
        openSelected(true);
    }
    if (!playing || (int32_t)(millis()-nextFrameAt) < 0) { delay(1); return; }
    if (restartPending) {
        openSelected(false);
        if (!playing) return;
    }
    uint32_t started = millis();
    canvas.startDecode();
    int duration = 0;
    int more = decoder.playFrame(false, &duration);
    if (pendingStep || requestRescan) { delay(1); return; }
    if (more < 0 || ioFault || canvas.invalid) {
        char note[31];
        snprintf(note, sizeof(note), "DECODER CODE %d", decoder.getLastError());
        failure(ioFault ? "SD READ ERROR" : "GIF DECODE ERROR", note);
        return;
    }
    if (canvas.sawLine) {
        present();
        ++framesPlayed;
    } else if (!framesPlayed) {
        failure("EMPTY GIF", "NO IMAGE FRAMES"); return;
    }
    restartPending = (more == 0);
    // Preserve the last frame's delay, but don't add a delay for trailing
    // metadata when the decoder reaches EOF without rendering another frame.
    nextFrameAt = started + playbackWaitMs(canvas.sawLine, duration);
    delay(1);
}
