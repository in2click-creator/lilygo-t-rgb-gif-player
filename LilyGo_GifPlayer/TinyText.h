#pragma once
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// Compact 5x7 uppercase status font. Animation does not contain UI overlays.
namespace TinyText {
static const char CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_:/?<> ";
static const uint8_t ROWS[][7] = {
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},
 {14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},{7,2,2,2,18,18,12},
 {17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},
 {14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},
 {17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},
 {14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
 {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},
 {14,17,17,14,17,17,14},{14,17,17,15,1,1,14},
 {0,0,0,0,0,6,6},{0,0,0,31,0,0,0},
 {0,0,0,0,0,0,31},{0,4,4,0,4,4,0},
 {1,2,2,4,8,8,16},{14,17,1,2,4,0,4},
 {2,4,8,16,8,4,2},{8,4,2,1,2,4,8},
 {0,0,0,0,0,0,0}
};
inline void centred(uint16_t *fb, int y, const char *text,
                    uint16_t colour = 0xffff, int scale = 2) {
    if (!fb || !text) return;
    char clean[31]; int len = 0;
    for (const unsigned char *p = (const unsigned char *)text; *p && len < 30; ++p) {
        if ((*p & 0xc0) == 0x80) continue; // one '?' per non-ASCII character
        clean[len++] = *p < 128 ? toupper(*p) : '?';
    }
    int x0 = (480 - len * 6 * scale + scale) / 2;
    for (int i = 0; i < len; ++i) {
        const char *entry = strchr(CHARS, clean[i]);
        int n = entry ? entry - CHARS : 41;
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col)
                if (ROWS[n][row] & (1 << (4-col)))
                    for (int yy = 0; yy < scale; ++yy)
                        for (int xx = 0; xx < scale; ++xx) {
                            int x = x0 + (i*6+col)*scale + xx;
                            int py = y + row*scale + yy;
                            if (x >= 0 && x < 480 && py >= 0 && py < 480)
                                fb[py*480+x] = colour;
                        }
    }
}
}
