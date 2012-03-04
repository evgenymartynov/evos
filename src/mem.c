#include "mem.h"
#include "printk.h"
#include "panic.h"
#include "monitor.h"
#include <mm/frame.h>

extern uint32_t __end;

uint32_t mem_first_unused;
uint32_t mem_total_bytes;

void init_mem(multiboot_info_t *mboot) {
    // Check if the mmap validity flag is set
    if (!(mboot->flags & (1 << 6))) {
        panic("multiboot's mmap is broken");
    }

    printk("Reading multiboot memory map...\n");
    // printk("    %-18s | %-18s | %-4s\n"
    //     "    -------------------+--------------------+-----\n",
    //     "   Base address",
    //     "   Region length",
    //     "Type"
    // );

    uint32_t base_addr = 0, region_len = 0;
    mem_total_bytes = 0;
    multiboot_memory_map_t *mmap;
    for (
        mmap = (multiboot_memory_map_t*)mboot->mmap_addr;
        (uint32_t)mmap < mboot->mmap_addr + mboot->mmap_length;
        mmap = (multiboot_memory_map_t*) \
            ((uint32_t)mmap + mmap->size + sizeof(mmap->size))
    ) {
        // printk("    %#08x%08x | %#08x%08x | %s\n",
        //     (uint32_t)(mmap->addr >> 32),
        //     (uint32_t)(mmap->addr & 0xFFFFFFFF),
        //     (uint32_t)(mmap->len >> 32),
        //     (uint32_t)(mmap->len & 0xFFFFFFFF),
        //     (mmap->type == 1) ? "free" : "used"
        // );

        // Check that the span fits in 32 bits
        if (mmap->addr >> 32 || (mmap->addr + mmap->len - 1) >> 32) {
            panic("Encountered a 64-bit memory region");
        }

        mem_total_bytes += mmap->len;

        if (mmap->type == 1) {
            uint32_t start = mmap->addr;        // Inclusive
            uint32_t end = start + mmap->len;   // Exclusive

            // Check that we are past the kernel image
            if (end >= (uint32_t)&__end) {
                // The image should fall into one of these regions
                // So we need to make sure we're not ignoring it
                // Otherwise we might find no usable memory
                if (start < (uint32_t)&__end) {
                    start = (uint32_t)&__end;
                }

                // And pick the largest contiguous region
                if (end-start > region_len) {
                    base_addr = start;
                    region_len = end-start;
                }
            }
        }
    }

    printk("  Found free memory at %#08x; size %d KB", base_addr, region_len/1024);
    mem_first_unused = base_addr;

    if (region_len > 16*1024*1024) {
        monitor_write_status("okay", 1);
    } else {
        panic("Insufficient memory");
    }

    // Proceed to set up the frame-allocator, paging, and the heap
    init_frames(mem_total_bytes);
}
