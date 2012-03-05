#include "kmalloc.h"
#include "stdint.h"
#include "stddef.h"
#include "kheap.h"
#include <mm/mem.h>
#include <mm/paging.h>

static uint32_t __linear_kmalloc(uint32_t size, int align, uint32_t *physical) {
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

static uint32_t __kmalloc(uint32_t size, int align, uint32_t *physical) {
    uint32_t new;
    if (kernel_heap) {
        extern page_directory_t *kernel_directory;
        new = (uint32_t)heap_alloc(kernel_heap, size, align);
        // TODO: this is utterly broken for *physical.
        // If the allocation does not span across consecutive regions,
        // the physical address will be only valid for the allocations
        // in the first page.
        // So like, ditch this shitty heap implementation and go for something
        // that allows contiguous chunks of the same physical memory to be used
        // This will probably require a double-layer memory architecture:
        //   1) Low-level frame-allocating interface with support for
        //      contiguous memory regions
        //   2) Higher-level heap-allocating interface which supports proper
        //      aligning and correct contiguous regions with physical addresses
        if (physical) {
            page_t *page = get_page(new, FALSE, kernel_directory);
            *physical = page->address * PAGE_SIZE +
                ((uint32_t)new & ~PAGE_ADDR_MASK);
        }

    } else  {
        new = __linear_kmalloc(size, align, physical);
    }

    return new;
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

// Free previously-kmalloc-ed memory
void kfree(void *ptr) {
    if (kernel_heap) {
        heap_free(kernel_heap, ptr);
    }
}
