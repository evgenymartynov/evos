#include "monitor.h"

static uint8_t *video_buffer = (uint8_t*)0xb8000;
static uint16_t cur_x, cur_y;

// TODO: define all the colours

void monitor_put(char value) {
    uint8_t fg_col = 15, bg_col = 0;
    uint16_t word = (bg_col << 4) | (fg_col & 0x0F);
    word <<= 8;

    // TODO: handle special chars here

    word |= value;

    uint16_t *location = (uint16_t*)(video_buffer) + (cur_y*80 + cur_x);
    *location = word;

    // TODO: handle lines
    cur_x++;

    // TODO: scroll if necessary
    // TODO: move the cursor
}
