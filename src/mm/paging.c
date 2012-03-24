#include "paging.h"
#include "panic.h"
#include "kmalloc.h"
#include "string.h"
#include "printk.h"
#include "mem.h"
#include "kheap.h"
#include "assert.h"

#include <mm/frame.h>

BUILD_BUG_ON_SIZEOF(page_t, 4);

#define CR0_PG_BIT  0x80000000

static void clone_page_to_physical(uint32_t src_virtual, uint32_t dest_physical);
static page_table_t* clone_table(int table_index, const page_table_t *src, uint32_t *physical);
static void invalidate_page(uint32_t address);

page_directory_t *kernel_directory  = 0;
page_directory_t *current_directory = 0;
static page_t *cloning_buffer_page = 0;

void page_alloc(page_t *page, int kernel_mode, int writeable) {
    if (page->address) {
        return;
    }

    uint32_t frame = frame_alloc();

    page->present = TRUE;
    page->writeable = writeable ? TRUE : FALSE;
    page->user_mode = TRUE; /* TODO */ //kernel_mode ? FALSE : TRUE;
    page->address = frame;   // Note that this is the top 20 bits
}

void page_free(page_t *page) {
    frame_free(page->address);
    page->address = 0;
    page->present = FALSE;
}

void init_paging(void) {
    uint32_t physical;
    kernel_directory = (page_directory_t*) kmalloc_ap(sizeof(page_directory_t), &physical);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    const uint32_t offset = (uint32_t)&kernel_directory->tables_physical_addr - (uint32_t)kernel_directory;
    kernel_directory->ismeta_tables_physical_addr = physical + offset;

    uint32_t i;
    // Prepare pages for the heap before identity-mapping the area
    // where these pages will reside.
    // At the same time, we need to kmalloc() those pages *before*
    // we stop identity-mapping things.
    for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += PAGE_SIZE) {
        get_page(i, TRUE, kernel_directory);
    }

    // Identity map whatever we have already used
    // and then some more - we need that to transition to kernel heap
    for (i = 0; i < mem_first_unused + PAGE_SIZE * 4; i += PAGE_SIZE) {
        page_t *this_page = get_page(i, TRUE, kernel_directory);
        page_alloc(this_page, FALSE, TRUE);

        // Check if someone called frame_alloc() before us.
        // If that is the case, identity mapping just screwed up.
        assert(this_page->address == i/PAGE_SIZE,
            "Cannot identity-map lower memory\n");
    }

    // Now *actually* map the heap to its pages
    for (i = KHEAP_START; i < KHEAP_START+KHEAP_INITIAL_SIZE; i += PAGE_SIZE) {
        page_alloc(get_page(i, FALSE, kernel_directory), FALSE, TRUE);
    }

    // And now set up the dud page for cloning
    cloning_buffer_page = get_page(PAGE_CLONING_BUFFER, TRUE, kernel_directory);
    page_alloc(cloning_buffer_page, TRUE, TRUE);

    printk("Enabling paging");
    isr_register_handler(PAGE_FAULT_ISR, page_fault_handler);
    switch_page_directory(kernel_directory);
    report_success();

    // And set up the heap.
    printk("Setting up the heap");
    kernel_heap = heap_create(KHEAP_START,
        KHEAP_START+KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS, FALSE, TRUE);
    report_success();

    // Just to make GCC shut up... and to test codes.
    printk("Trying to clone the page directory");
    switch_page_directory(clone_directory(kernel_directory));
    report_success();
}

page_t* get_page(uint32_t address, int create_missing, page_directory_t *dir) {
    address /= PAGE_SIZE;
    const uint32_t table_index = address / PAGES_PER_TABLE;
    const uint32_t page_index  = address % PAGES_PER_TABLE;

    if (dir->tables[table_index]) {
        return &dir->tables[table_index]->pages[page_index];
    } else if (create_missing) {
        uint32_t physical;

        dir->tables[table_index] =
            (page_table_t*)kmalloc_ap(sizeof(page_table_t), &physical);
        memset(dir->tables[table_index], 0, sizeof(page_table_t));

        dir->tables_physical_addr[table_index] =
            physical | 7;   // present, user, writeable

        return &dir->tables[table_index]->pages[page_index];
    } else {
        return 0;
    }
}

page_directory_t *clone_directory(page_directory_t *src) {
    uint32_t physical;
    page_directory_t *dir = (page_directory_t*)kmalloc_ap(sizeof(*dir), &physical);
    memset(dir, 0, sizeof(*dir));

    const uint32_t offset = (uint32_t)&dir->tables_physical_addr - (uint32_t)dir;
    dir->ismeta_tables_physical_addr = physical + offset;

    int i;
    for (i = 0; i < TABLES_PER_DIRECTORY; i++) {
        if (src->tables[i]) {
            if (kernel_directory->tables[i] == src->tables[i]) {
                dir->tables[i] = src->tables[i];
                dir->tables_physical_addr[i] = src->tables_physical_addr[i];
            } else {
                printk("Clone!\n");
                uint32_t physical;
                dir->tables[i] = clone_table(i, (page_table_t*)src->tables[i], &physical);
                dir->tables_physical_addr[i] = physical | 7;
            }
        }
    }

    return dir;
}

static page_table_t* clone_table(int table_index, const page_table_t *src, uint32_t *physical) {
    page_table_t *dest = (page_table_t*)kmalloc_ap(sizeof(*dest), physical);
    memset(dest, 0, sizeof(*dest));

    int i;
    for (i = 0; i < PAGES_PER_TABLE; i++) {
        const page_t *sp = &src->pages[i];
        page_t *dp = &dest->pages[i];
        if (sp->address) {
            page_alloc(dp, !sp->user_mode, sp->writeable);
            uint32_t page_index = table_index * PAGES_PER_TABLE + i;
            clone_page_to_physical(page_index * PAGE_SIZE, dp->address * PAGE_SIZE);
        }
    }

    return dest;
}

// Call INVLPG on a given address.
static void invalidate_page(uint32_t address) {
    asm volatile("invlpg %0" :: "m"((void*)address));

    // TODO: INVLPG does not work :(
    asm volatile (
        "xorl %eax, %eax;"
        "movl %cr3, %eax;"
        "movl %eax, %cr3;"
    );
}

// Given a pair of addresses, copy one page onto another.
// This uses a dud page - maybe not the best idea, but it works.
// Takes a virtual src address and a physical destination address.
// Assumes that given src is accessible in current virtual address space.
static void clone_page_to_physical(uint32_t src_virtual, uint32_t dest_physical) {
    cloning_buffer_page->address = dest_physical / PAGE_SIZE;
    cloning_buffer_page->present = TRUE;
    invalidate_page(PAGE_CLONING_BUFFER);

    memcpy((void*)PAGE_CLONING_BUFFER, (void*)src_virtual, PAGE_SIZE);

    cloning_buffer_page->present = FALSE;
    invalidate_page(PAGE_CLONING_BUFFER);
}

void switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    asm volatile ("movl %0, %%cr3" : : "r"(dir->ismeta_tables_physical_addr));

    uint32_t cr0;
    asm volatile ("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= CR0_PG_BIT;
    asm volatile ("movl %0, %%cr0" : : "r"(cr0));
}

void page_fault_handler(registers_t *regs) {
    uint32_t fault_address;
    asm volatile ("movl %%cr2, %0" : "=r"(fault_address));

    int present =   (regs->err_code & 1);
    int on_write =  (regs->err_code & 2);
    int user_mode = (regs->err_code & 4);
    int reserved =  (regs->err_code & 8);
    int op_fetch =  (regs->err_code & 16);

    printk("Page-fault: page %spresent; failed on %s in %s-mode at 0x%08x\n",
        present     ? ""        : "NOT "    ,
        on_write    ? "write"   : "read"    ,
        user_mode   ? "user"    : "kernel"  ,
        fault_address
    );
    if (reserved) printk("Also trampled reserved bits\n");
    if (op_fetch) printk("Error occurred during op-fetch\n");
    printk("At eip=%p\n", regs->eip);

    panic("Page fault!");
}
