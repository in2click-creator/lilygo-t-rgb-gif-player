#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include "src/AnimatedGIF/AnimatedGIF.h"

// Compositing is performed in screen coordinates. Only the centre crop is
// stored, including for GIF disposal methods 2 and 3. Source frames can be
// much larger than 480x480 without allocating a source-sized framebuffer.
class GifCanvas {
public:
    static constexpr int SIZE = 480;
    static constexpr size_t PIXELS = SIZE * SIZE;
    uint16_t *pixels = nullptr;
    uint16_t *saved = nullptr;
    bool sawLine = false;
    bool invalid = false;

    void configure(int w, int h, uint16_t bg) {
        width = w; height = h; background = bg;
        int side = w < h ? w : h;
        // Half-pixel-centred nearest-neighbour sampling, with a centre crop.
        for (int i = 0; i < SIZE; ++i) {
            xMap[i] = ((int64_t)(w - side) * SIZE + (2*i+1)*(int64_t)side) / (2*SIZE);
            yMap[i] = ((int64_t)(h - side) * SIZE + (2*i+1)*(int64_t)side) / (2*SIZE);
        }
        previous = Rect{};
        firstFrame = true;
        std::fill_n(pixels, PIXELS, background);
    }

    void startDecode() { sawLine = false; invalid = false; }

    void draw(const GIFDRAW &d) {
        if (invalid) return;
        if (d.iX < 0 || d.iY < 0 || d.iWidth <= 0 || d.iHeight <= 0 ||
            (int64_t)d.iX + d.iWidth > width || (int64_t)d.iY + d.iHeight > height ||
            d.y < 0 || d.y >= d.iHeight || !d.pPixels || !d.pPalette) {
            invalid = true;
            return;
        }
        if (!sawLine) {
            if (firstFrame) {
                // Transparent portions are composited onto black.
                std::fill_n(pixels, PIXELS, d.ucHasTransparency ? 0 : background);
                firstFrame = false;
            } else if (previous.disposal == 2) {
                fillSourceRect(previous, previous.transparent ? 0 : background);
            } else if (previous.disposal == 3) {
                memcpy(pixels, saved, PIXELS * sizeof(uint16_t));
            }
            if (d.ucDisposalMethod == 3)
                memcpy(saved, pixels, PIXELS * sizeof(uint16_t));
            previous = {d.iX, d.iY, d.iWidth, d.iHeight,
                        d.ucDisposalMethod, d.ucHasTransparency != 0};
            sawLine = true;
        }
        const int sy = d.iY + d.y;
        int y0 = lower(yMap, sy), y1 = lower(yMap, sy + 1);
        if (y0 == y1) return; // This source row is outside the sampled crop.
        int x0 = lower(xMap, d.iX), x1 = lower(xMap, d.iX + d.iWidth);
        for (int y = y0; y < y1; ++y) {
            uint16_t *row = pixels + y * SIZE;
            for (int x = x0; x < x1; ++x) {
                uint8_t index = d.pPixels[xMap[x] - d.iX];
                if (!d.ucHasTransparency || index != d.ucTransparent)
                    row[x] = d.pPalette[index];
            }
        }
    }

private:
    struct Rect {
        int x, y, w, h;
        uint8_t disposal;
        bool transparent;
        Rect(int x_=0, int y_=0, int w_=0, int h_=0, uint8_t d_=0, bool t_=false)
            : x(x_), y(y_), w(w_), h(h_), disposal(d_), transparent(t_) {}
    };
    int width = 0, height = 0;
    int xMap[SIZE], yMap[SIZE];
    uint16_t background = 0;
    Rect previous;
    bool firstFrame = true;
    static int lower(const int *map, int v) {
        return std::lower_bound(map, map + SIZE, v) - map;
    }
    void fillSourceRect(const Rect &r, uint16_t colour) {
        int x0 = lower(xMap, r.x), x1 = lower(xMap, r.x + r.w);
        int y0 = lower(yMap, r.y), y1 = lower(yMap, r.y + r.h);
        for (int y = y0; y < y1; ++y)
            std::fill(pixels + y*SIZE + x0, pixels + y*SIZE + x1, colour);
    }
};
