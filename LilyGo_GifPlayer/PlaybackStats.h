#pragma once
#include <stdint.h>

struct TimingSamples {
    uint64_t totalUs = 0;
    uint32_t count = 0, maxUs = 0;
    void add(uint32_t us) {
        totalUs += us;
        ++count;
        if (us > maxUs) maxUs = us;
    }
    double meanMs() const { return count ? totalUs / (1000.0 * count) : 0.0; }
};

struct PlaybackStats {
    TimingSamples decode, output, reopen;
    uint32_t sinceMs = 0, overBudget = 0;
    void reset(uint32_t now) { *this = PlaybackStats{}; sinceMs = now; }
    void frame(uint32_t decodeUs, uint32_t outputUs, uint32_t budgetMs) {
        decode.add(decodeUs);
        output.add(outputUs);
        if ((uint64_t)decodeUs + outputUs > (uint64_t)budgetMs * 1000)
            ++overBudget;
    }
    uint32_t elapsed(uint32_t now) const { return now - sinceMs; }
    double fps(uint32_t now) const {
        return elapsed(now) ? decode.count * 1000.0 / elapsed(now) : 0.0;
    }
};
