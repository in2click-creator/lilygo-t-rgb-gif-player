#include <cassert>
#include "../LilyGo_GifPlayer/PlaybackTiming.h"

int main() {
    assert(playbackWaitMs(false, 0) == 0);
    assert(playbackWaitMs(false, 200) == 0);
    assert(playbackWaitMs(true, 0) == 100);
    assert(playbackWaitMs(true, -1) == 100);
    assert(playbackWaitMs(true, 10) == 20);
    assert(playbackWaitMs(true, 20) == 20);
    assert(playbackWaitMs(true, 200) == 200);
}
