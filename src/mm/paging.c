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

page_directory_t *kernel_directory  = 0;
page_directory_t *current_directory = 0;

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
    kernel_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));

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

    printk("Setting up paging");
    isr_register_handler(PAGE_FAULT_ISR, page_fault_handler);
    switch_page_directory(kernel_directory);
    report_success();

    // And set up the heap.
    printk("Setting up the heap");
    kernel_heap = heap_create(KHEAP_START,
        KHEAP_START+KHEAP_INITIAL_SIZE, KHEAP_MAX_ADDRESS, FALSE, TRUE);
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

void switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    asm volatile ("movl %0, %%cr3" : : "r"(&dir->tables_physical_addr));

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
