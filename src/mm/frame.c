#include "frame.h"
#include "mem.h"
#include <datastructs/bitset.h>
#include "panic.h"

static bitset_t frames; // TODO allocate
static uint32_t num_frames;

void init_frames(uint32_t total_memory) {
    num_frames = total_memory / PAGE_SIZE;
    bitset_create(&frames, num_frames);
}

uint32_t frame_alloc(void) {
    uint32_t frame = bitset_find_free(&frames);
    if (frame == (uint32_t)-1) {
        panic("No free frames\n");
    }

    bitset_set(&frames, frame);
    return frame;
}

void frame_free(uint32_t frame) {
    if (frame) {
        bitset_clear(&frames, frame);
    }
}
