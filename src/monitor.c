#include "monitor.h"
#include "stddef.h"

static uint8_t *video_buffer = (uint8_t*)0xb8000;
static uint16_t cur_x, cur_y;

// TODO: define all the colours

static void move_cursor() {
    uint16_t position = 80*cur_y + cur_x;
    outb(0x3D4, 14);
    outb(0x3D5, position >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, position & 0xFF);
}

void monitor_put(char value) {
    uint8_t fg_col = 15, bg_col = 0;
    uint16_t word = (bg_col << 4) | (fg_col & 0x0F);
    word <<= 8;

    // TODO: handle special chars here

    word |= value;

    uint16_t *location = (uint16_t*)(video_buffer) + (cur_y*80 + cur_x);
    *location = word;

    cur_x++;
    if (cur_x == 80) {
        cur_x = 0;
        cur_y++;
    }

    // TODO: scroll if necessary
    move_cursor();
}

void monitor_clear() {
    int i;
    for (i = 0; i < 80*25; i += 2) {
        video_buffer[i  ] = ' ';
        video_buffer[i+1] = 0x0F;
    }
    move_cursor();
}

void monitor_write(char *str) {
    while (*str) {
        monitor_put(*str);
        str++;
    }
}

void monitor_write_hex(uint32_t value) {
    int i;
    const char letters[] = "0123456789abcdef";

    for (i = sizeof(value)*8-4; i >= 0; i -= 4) {
        uint8_t nibble = (value & (0x0F << i)) >> i;
        monitor_put(letters[nibble]);
    }
}

void monitor_write_dec(uint32_t value) {
    int i;
    int had_digit = FALSE;

    for (i = 1000000000; i >= 1; i /= 10) {
        uint32_t digit = (value / i) % 10;

        if (digit == 0 && !had_digit && i != 1)
            continue;

        monitor_put('0' + digit);
        had_digit = TRUE;
    }
}
