#include "bitset.h"
#include "stddef.h"
#include "string.h"
#include "kmalloc.h"

#define BITS_PER_ITEM   (sizeof(uint32_t) * 8)
#define OUTER_INDEX(i)  (i / BITS_PER_ITEM)
#define INNER_INDEX(i)  (i % BITS_PER_ITEM)
#define ALL_USED        0xFFFFFFFF

void bitset_create(Bitset set, uint32_t size) {
    set->size = (size + BITS_PER_ITEM - 1) / BITS_PER_ITEM;
    set->base = (uint32_t*)kmalloc(set->size);
    memset(set->base, 0, set->size);
}

void bitset_set(Bitset set, uint32_t index) {
    const uint32_t outer = OUTER_INDEX(index);
    const uint32_t inner = INNER_INDEX(index);

    if (outer >= set->size) {
        return;
    }

    set->base[outer] |= (1 << inner);
}

void bitset_clear(Bitset set, uint32_t index) {
    const uint32_t outer = OUTER_INDEX(index);
    const uint32_t inner = INNER_INDEX(index);

    if (outer >= set->size) {
        return;
    }

    set->base[outer] &= ~(1 << inner);
}

int bitset_check_set(Bitset set, uint32_t index) {
    const uint32_t outer = OUTER_INDEX(index);
    const uint32_t inner = INNER_INDEX(index);

    if (outer >= set->size) {
        return FALSE;
    }


    return (set->base[outer] & (1 << inner));
}

uint32_t bitset_find_free(Bitset set) {
    uint32_t outer, inner;
    for (outer = 0; outer < set->size; outer++) {
        if (set->base[outer] != ALL_USED) {
            for (inner = 0; inner < BITS_PER_ITEM; inner++) {
                uint32_t index = outer*BITS_PER_ITEM + inner;
                if (!bitset_check_set(set, index)) {
                    return index;
                }
            }
        }
    }

    return (uint32_t) -1;
}
