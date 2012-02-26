#include "kheap.h"
#include "stddef.h"
#include "assert.h"
#include "mem.h"
#include "paging.h"
#include "kmalloc.h"
#include "ordered_array.h"

#include "printk.h"
#include "string.h"

typedef struct {
    uint32_t magic;
    int is_hole;
    uint32_t size;
} header_t;

typedef struct {
    uint32_t magic;
    header_t *header;
} footer_t;

extern page_directory_t *kernel_directory;
heap_t *kernel_heap;

static int header_t_cmp(void *_a, void *_b) {
    header_t *a = (header_t*)_a;
    header_t *b = (header_t*)_b;

    return a->size < b->size;
}

// Return the offset from addr, such that adding it to addr
// will produce a hole with the page-aligned usable region.
// Also take care with header_t and footer_t - they must fit in the offset.
static uint32_t page_align_get_offset(uint32_t addr) {
    addr += sizeof(header_t);
    if (addr & ~PAGE_ADDR_MASK) {
        uint32_t offset = PAGE_SIZE - (addr & ~PAGE_ADDR_MASK);

        // Make sure we are able to split this hole into two
        if (offset <= sizeof(header_t) + sizeof(footer_t)) {
            offset += PAGE_SIZE;
        }

        return offset;
    } else {
        return 0;
    }
}

// Finds smallest hole in the heap that will fit
// Returns -1 (0xff...) on failure
static uint32_t find_hole(heap_t *heap, uint32_t size, int page_align) {
    uint32_t i;
    int found = FALSE;

    for (i = 0; i < heap->index.size; i++) {
        header_t *header = (header_t*)ordered_array_index(&heap->index, i);
        if (page_align) {
            // We need to page-align
            uint32_t offset = page_align_get_offset((uint32_t)header);

            // After page-aligning, we might have to skip the first few
            // bytes of this hole. Check if it is sufficiently large.
            if ((int32_t)header->size - (int32_t)offset >= (int32_t)size) {
                found = TRUE;
                break;
            }
        } else if (header->size >= size) {
            found = TRUE;
            break;
        }
    }

    if (found) {
        return i;
    } else {
        return -1;
    }
}

static void expand(heap_t *heap, uint32_t new_size) {
    if (new_size & ~PAGE_ADDR_MASK) {
        new_size &= PAGE_ADDR_MASK;
        new_size += PAGE_SIZE;
    }

    // This is always page-aligned ^_^
    const uint32_t old_size = heap->allocated_end - heap->allocated_start;

    assert(new_size > old_size, "Tried to expand heap to a smaller size");
    assert(heap->allocated_start + new_size <= heap->max_addr,
        "Cannot expand heap: not enough free space available");

    uint32_t i;
    for (i = old_size; i < new_size; i += PAGE_SIZE) {
        alloc_frame(get_page(heap->allocated_start + i, TRUE, kernel_directory),
            !heap->pages_user_mode, heap->pages_writeable);
    }

    heap->allocated_end = heap->allocated_start + new_size;
}

// static void contract(heap_t *heap, uint32_t new_size) {
//     if (new_size & ~PAGE_ADDR_MASK) {
//         new_size &= PAGE_ADDR_MASK;
//         new_size += PAGE_SIZE;
//     }

//     if (new_size < KHEAP_MINIMUM_SIZE) {
//         new_size = KHEAP_MINIMUM_SIZE;
//     }

//     // This is always page-aligned ^_^
//     const uint32_t old_size = heap->allocated_end - heap->allocated_start;

//     assert(new_size < old_size, "Tried to contract heap to a larger size");
//     uint32_t i;
//     for (i = old_size - PAGE_SIZE; new_size < i; i -= PAGE_SIZE) {
//         free_frame(get_page(heap->allocated_start + i, FALSE, kernel_directory));
//     }

//     heap->allocated_end = heap->allocated_start + new_size;
// }

heap_t* heap_create(uint32_t start, uint32_t end, uint32_t max_addr,
    int pages_user_mode, int pages_writeable) {
    heap_t *heap = (heap_t*)kmalloc(sizeof(*heap));

    assert(!(start & ~PAGE_ADDR_MASK), "Heap start address is not page-aligned");
    assert(!(end   & ~PAGE_ADDR_MASK), "Heap end   address is not page-aligned");

    // Create the index at the beginning of the heap
    heap->index = ordered_array_create_at((void*)start,
        KHEAP_INDEX_SIZE, &header_t_cmp);

    // Page-align the heap body
    start += sizeof(type_t) * KHEAP_INDEX_SIZE;
    if (start & ~PAGE_ADDR_MASK) {
        start &= PAGE_ADDR_MASK;
        start += PAGE_SIZE;
    }

    // Set stuff up
    heap->allocated_start = start;
    heap->allocated_end = end;
    heap->max_addr = max_addr;
    heap->pages_user_mode = pages_user_mode;
    heap->pages_writeable = pages_writeable;

    // Create one large hole in the index
    header_t *header = (header_t*)start;
    header->size = end - start;
    header->magic = KHEAP_MAGIC;
    header->is_hole = TRUE;
    ordered_array_insert(&heap->index, (void*)header);

    return heap;
}

void* heap_alloc(heap_t *heap, uint32_t __size, int page_align) {
    uint32_t new_size = __size + sizeof(header_t) + sizeof(footer_t);
    uint32_t hole_index = find_hole(heap, new_size, page_align);

    if (hole_index == -1) {
        // Save stuff
        uint32_t old_length = heap->allocated_end - heap->allocated_start;
        uint32_t old_allocated_end = heap->allocated_end;

        // Expand the heap. This will automagically page-align.
        expand(heap, old_length + new_size);
        uint32_t new_length = heap->allocated_end - heap->allocated_start;

        // Create the hole that we just allocated
        header_t *new_header = (header_t*)old_allocated_end;
        new_header->size = new_length - old_length;
        new_header->magic = KHEAP_MAGIC;
        new_header->is_hole = TRUE;
        footer_t *new_footer = (footer_t*) \
            (heap->allocated_end - sizeof(*new_footer));
        new_footer->magic = KHEAP_MAGIC;
        new_footer->header = new_header;

        ordered_array_insert(&heap->index, (void*)new_header);
        hole_index = find_hole(heap, new_size, page_align);

        assert(hole_index != -1, "Failed to find newly allocated hole");
    }

    // Save stuff
    header_t *orig_hole_header = \
        (header_t*)ordered_array_index(&heap->index, hole_index);
    uint32_t orig_hole_addr = (uint32_t)orig_hole_header;
    uint32_t orig_hole_size = orig_hole_header->size;

    // Work out if we can *NOT* split the hole in two
    // If that is the case, extend the allocation a bit
    if (orig_hole_size - new_size <= sizeof(header_t) + sizeof(footer_t)) {
        __size += orig_hole_size - new_size;
        new_size = orig_hole_size;
    }

    // If we are told to page align, make a hole in front of the block
    if (page_align && (orig_hole_addr & ~PAGE_ADDR_MASK)) {
        uint32_t offset = page_align_get_offset(orig_hole_addr);
        uint32_t new_addr = orig_hole_addr + offset;

        // This should never occur
        assert(offset > sizeof(header_t) + sizeof(footer_t),
            "Hit a known bug in heap_alloc() due to page-aligning.");

        // Allocate stuff for the hole on the left
        header_t *left_header = (header_t*)orig_hole_addr;
        left_header->size = offset;
        left_header->magic = KHEAP_MAGIC;
        left_header->is_hole = TRUE;
        footer_t *left_footer = (footer_t*)(new_addr - sizeof(*left_footer));
        left_footer->magic = KHEAP_MAGIC;
        left_footer->header = left_header;

        // Adjust the hole on the right (now page-aligned)
        orig_hole_addr = new_addr;
        orig_hole_size          -= offset;
        orig_hole_header->size  -= offset;

        // Yet another contradiction with the tutorial here... Sigh.
        // Remove the entry and re-insert it to keep the sorting invariant.
        ordered_array_remove(&heap->index, hole_index);
        ordered_array_insert(&heap->index, (void*)orig_hole_header);
    } else {
        // Don't need to page-align; we are going to use this hole now
        ordered_array_remove(&heap->index, hole_index);
    }

    // And *now* we create the new block
    // At this point, only orig_hole_addr and orig_hole_size are valid
    // The rest are not guaranteed, notably orig_hole_header and hole_index

    // Set up footer/header stuffs for the allocated block
    header_t *block_header = (header_t*)orig_hole_addr;
    block_header->size = new_size;
    block_header->magic = KHEAP_MAGIC;
    block_header->is_hole = FALSE;
    footer_t *block_footer = \
        (footer_t*)(orig_hole_addr + new_size - sizeof(*block_footer));
    block_footer->magic = KHEAP_MAGIC;
    block_footer->header = block_header;

    assert((uint32_t)block_footer < heap->allocated_end,
            "Fell over the edge of the heap");

    // Check if we have free space on the right
    if (orig_hole_size - new_size > 0) {
        assert(orig_hole_size - new_size > sizeof(header_t) + sizeof(footer_t),
            "We have free space in the hole, but not enough to make a new one");

        header_t *right_header = (header_t*)(orig_hole_addr + new_size);
        right_header->size = orig_hole_size - new_size;
        right_header->magic = KHEAP_MAGIC;
        right_header->is_hole = TRUE;
        footer_t *right_footer = (footer_t*) \
            (orig_hole_addr + orig_hole_size - sizeof(*right_footer));
        right_footer->magic = KHEAP_MAGIC;
        right_footer->header = right_header;

        assert((uint32_t)right_footer < heap->allocated_end,
            "Fell over the edge of the heap");

        ordered_array_insert(&heap->index, (void*)right_header);
    }

    return (void*)((uint32_t)block_header + sizeof(*block_header));
}
