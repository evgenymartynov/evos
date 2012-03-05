#ifndef _KHEAP_H_
#define _KHEAP_H_

#include "stddef.h"
#include <datastructs/ordered_array.h>

#define KHEAP_START         0x0f000000
#define KHEAP_MAX_ADDRESS   0x0ffff000
#define KHEAP_INITIAL_SIZE  0x00010000
#define KHEAP_MINIMUM_SIZE  0x00001000
#define KHEAP_INDEX_SIZE    (KHEAP_INITIAL_SIZE / 32)
#define KHEAP_MAGIC         0xDEADBEEF

typedef struct {
    ordered_array_t index;
    uint32_t allocated_start;
    uint32_t allocated_end;
    uint32_t max_addr;
    int pages_user_mode;
    int pages_writeable;
} heap_t;

extern heap_t *kernel_heap;

heap_t* heap_create(uint32_t start, uint32_t end, uint32_t max_addr,
    int pages_user_mode, int pages_writeable);

void* heap_alloc(heap_t *heap, uint32_t size, int page_align);
void heap_free(heap_t *heap, void *ptr);

#endif
