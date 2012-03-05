#ifndef _ORDERED_ARRAY_H_
#define _ORDERED_ARRAY_H_

#include "stdint.h"

typedef void *type_t;
typedef int (*less_than_cmp)(type_t, type_t);

typedef struct {
    type_t *array;
    uint32_t size, max_size;
    less_than_cmp comparator;
} ordered_array_t;

ordered_array_t ordered_array_create(uint32_t size, less_than_cmp cmp);
ordered_array_t ordered_array_create_at(void *where, uint32_t size, \
    less_than_cmp cmp);

void ordered_array_insert(ordered_array_t *this, type_t value);
void ordered_array_remove(ordered_array_t *this, uint32_t index);
type_t ordered_array_index(ordered_array_t *this, uint32_t index);
void ordered_array_changed_element(ordered_array_t *this);

#endif
