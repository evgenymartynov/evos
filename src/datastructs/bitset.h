#ifndef _BITSET_H_
#define _BITSET_H_

#include "stddef.h"

typedef struct {
    uint32_t *base;
    uint32_t size;
} bitset_t;

typedef bitset_t *Bitset;

void bitset_create(Bitset set, uint32_t size);

void bitset_set(Bitset set, uint32_t index);
void bitset_clear(Bitset set, uint32_t index);
int bitset_check_set(Bitset set, uint32_t index);
uint32_t bitset_find_free(Bitset set);

#endif