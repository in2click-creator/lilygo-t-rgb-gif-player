#pragma once
#include <stdint.h>

// An end-of-stream call may consume trailing metadata without drawing a frame.
// Only displayed frames get a delay; otherwise restart on the next loop tick.
inline uint32_t playbackWaitMs(bool renderedFrame, int duration) {
    if (!renderedFrame) return 0;
    if (duration <= 0) return 100;
    return duration < 20 ? 20u : (uint32_t)duration;
}
