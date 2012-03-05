#include "monitor.h"
#include "stddef.h"
#include "string.h"

static uint16_t *video_buffer = (uint16_t*)0xb8000;
static uint16_t cur_x, cur_y;

// Generates a framebuffer word
#define FB_WORD(ch, at) ( ( (uint16_t)(at) << 8 ) | (uint16_t)(ch) )
// Generates a framebuffer colour attribute
#define FB_ATTR(fg, bg) ( ( (uint8_t )(bg) << 4 ) | (uint8_t )(fg) )

#define COL_WHITE 15
#define COL_LTGREY 7
#define COL_RED   4
#define COL_GREEN 2
#define COL_BLACK 0
// TODO: define all the colours

static const uint16_t BLANK = FB_WORD(' ', FB_ATTR(COL_LTGREY, COL_BLACK));

static void move_cursor() {
    uint16_t position = 80*cur_y + cur_x;
    outb(0x3D4, 14);
    outb(0x3D5, position >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, position & 0xFF);
}

static void scroll() {
    if (cur_y == 25) {
        int i;

        for (i = 0; i < 80*24; i++) {
            video_buffer[i] = video_buffer[i+80];
        }

        for (; i < 80*25; i++) {
            video_buffer[i] = BLANK;
        }

        cur_y = 24;
    }
}

static void __monitor_put(char value, uint8_t attributes) {
    if (value == 0x08) {
        // backspace
        if (cur_x) {
            cur_x--;
        }
    } else if (value == '\t') {
        cur_x = (cur_x + 8) & ~7;
    } else if (value == '\r') {
        cur_x = 0;
    } else if (value == '\n') {
        cur_y++;
        cur_x = 0;
    } else {
        uint16_t word = FB_WORD(value, attributes);
        uint16_t *location = video_buffer + (cur_y*80 + cur_x);
        *location = word;
        cur_x++;
    }

    if (cur_x >= 80) {
        cur_x = 0;
        cur_y++;
    }

    scroll();
    move_cursor();
}

void monitor_put(char value) {
    __monitor_put(value, FB_ATTR(COL_LTGREY, COL_BLACK));
}

void monitor_clear() {
    int i;
    uint16_t *vid_mem = video_buffer;
    for (i = 0; i < 80*25; i++) {
        vid_mem[i] = BLANK;
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

void monitor_write_status(const char *str, int success) {
    uint8_t attr = success
                   ? FB_ATTR(COL_GREEN, COL_BLACK)
                   : FB_ATTR(COL_RED,   COL_BLACK);

    int len = strlen(str) + 2; // for []
    cur_x = 80 - len;

    monitor_put('[');
    while (*str) {
        __monitor_put(*str, attr);
        str++;
    }
    monitor_put(']');
}
