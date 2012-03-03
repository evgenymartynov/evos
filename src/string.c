#include "string.h"

void memset(void *region, uint8_t value, uint32_t size) {
    uint8_t *ptr = (uint8_t*)region;
    for (; size; size--) {
        *ptr++ = value;
    }
}

void memcpy(void *_dest, void *_src, uint32_t num) {
    char *dest = _dest, *src = _src;
    while (num--) {
        *dest++ = *src++;
    }
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int strlen(const char *string) {
    int len = 0;
    for (; *string; string++) {
        len++;
    }

    return len;
}

int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }

    return (int)*b - (int)*a;
}
