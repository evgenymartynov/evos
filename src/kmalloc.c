#include "kmalloc.h"
#include "mem.h"
#include "stdint.h"

#define PAGE_SIZE       4096        // TODO: defined in 2 places
#define PAGE_ADDR_MASK  0xFFFFF000  // Upper bits of page-aligned addresses

static uint32_t __kmalloc(uint32_t size, int align, uint32_t *physical) {
    if (align && (mem_first_unused & ~PAGE_ADDR_MASK)) {
        mem_first_unused &= PAGE_ADDR_MASK;
        mem_first_unused += PAGE_SIZE;
    }

    uint32_t res = mem_first_unused;
    mem_first_unused += size;

    if (physical) {
        *physical = res;
    }

    return res;
}

// Plain kmalloc
uint32_t kmalloc(uint32_t size) {
    return __kmalloc(size, 0, 0);
}

// Align allocation
uint32_t kmalloc_a(uint32_t size) {
    return __kmalloc(size, 1, 0);
}

// Return physical address
uint32_t kmalloc_p(uint32_t size, uint32_t *physical) {
    return __kmalloc(size, 0, physical);
}

// Align and return physical address
uint32_t kmalloc_ap(uint32_t size, uint32_t *physical) {
    return __kmalloc(size, 1, physical);
}
