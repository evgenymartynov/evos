#include "string.h"

void memset(void *region, uint8_t value, uint32_t size) {
    uint8_t *ptr = (uint8_t*)region;
    for (; size; size--) {
        *ptr++ = value;
    }
}
