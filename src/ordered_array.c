#include "ordered_array.h"
#include "string.h"
#include "kmalloc.h"
#include "panic.h"

static int standard_less_than_cmp(type_t a, type_t b) {
    return (uint32_t)a < (uint32_t)b;
}

ordered_array_t ordered_array_create(uint32_t size, less_than_cmp cmp) {
    return ordered_array_create_at( \
        (void*)kmalloc(sizeof(type_t) * size), size, cmp);
}

ordered_array_t ordered_array_create_at(void *where, uint32_t size, \
    less_than_cmp cmp) {
    if (!cmp) {
        cmp = standard_less_than_cmp;
    }

    ordered_array_t array;
    array.array = where;
    memset(array.array, 0, sizeof(type_t) * size);
    array.comparator = cmp;
    array.size = 0;
    array.max_size = size;

    return array;
}

void ordered_array_insert(ordered_array_t *this, type_t value) {
    if (this->size + 1 > this->max_size) {
        panic("Tried to insert into a full ordered_array_t");
    }

    this->array[this->size++] = value;
    ordered_array_changed_element(this);
}

void ordered_array_remove(ordered_array_t *this, uint32_t index) {
    if (index >= this->size) {
        panic("Array index out of bounds");
    }

    int i = index;
    this->size--;
    for (; i < (int)this->size; i++) {
        this->array[i] = this->array[i+1];
    }
}

type_t ordered_array_index(ordered_array_t *this, uint32_t index) {
    if (index >= this->size) {
        panic("Array index out of bounds");
    }

    return this->array[index];
}

// One element was changed and the sequence needs updating
void ordered_array_changed_element(ordered_array_t *this) {
    int i;
    for (i = this->size-1; i > 0; i--) {
        if (this->comparator(this->array[i], this->array[i-1])) {
            type_t temp = this->array[i];
            this->array[i] = this->array[i-1];
            this->array[i-1] = temp;
        }
    }
}
