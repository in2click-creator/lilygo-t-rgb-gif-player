#include <cassert>
#include <cmath>
#include "../LilyGo_GifPlayer/PlaybackStats.h"

int main() {
    PlaybackStats s;
    s.reset(100);
    s.frame(5000, 3000, 20);
    s.frame(18000, 3000, 20);
    s.frame(17000, 3000, 20); // Exactly on budget is not over budget.
    s.reopen.add(4000);
    s.reopen.add(8000);
    assert(s.overBudget == 1 && s.decode.count == 3);
    assert(s.decode.maxUs == 18000 && s.output.maxUs == 3000);
    assert(std::abs(s.decode.meanMs() - 40.0 / 3) < 0.0001);
    assert(s.reopen.meanMs() == 6 && s.reopen.maxUs == 8000);
    assert(s.fps(1100) == 3);
    s.reset(UINT32_MAX - 99);
    assert(s.elapsed(100) == 200); // millis() rollover.
    assert(s.decode.count == 0 && s.reopen.count == 0 && s.overBudget == 0);
    assert(s.fps(100) == 0 && s.decode.meanMs() == 0);
    s.frame(UINT32_MAX, 1000, 20); // Summing timings must not overflow.
    assert(s.overBudget == 1);
}
