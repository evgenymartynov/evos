#include "paging.h"
#include "stddef.h"
#include "panic.h"
#include "bitset.h"
#include "kmalloc.h"
#include "string.h"
#include "printk.h"

BUILD_BUG_ON_SIZEOF(page_t, 4);

#define PAGE_SIZE   4096    // TODO: defined in 2 places
#define CR0_PG_BIT  0x80000000

extern uint32_t kmalloc_first_unused;

static Bitset   frames; // TODO allocate
static uint32_t num_frames;

page_directory_t *kernel_directory  = 0;
page_directory_t *current_directory = 0;

static void alloc_frame(page_t *page, int kernel_mode, int writeable) {
    if (page->address) {
        return;
    }

    uint32_t frame = bitset_find_free(frames);
    if (frame == (uint32_t)-1) {
        panic("No free frames\n");
    }

    bitset_set(frames, frame);

    page->present = TRUE;
    page->writeable = writeable ? TRUE : FALSE;
    page->user_mode = kernel_mode ? FALSE : TRUE;
    page->address = frame;   // Note that this is the top 20 bits
}

#if 0
static void free_frame(page_t *page) {
    uint32_t frame = page->address;
    if (frame) {
        bitset_clear(frames, frame);
        page->address = 0;
    }
}
#endif

void init_paging(void) {
    uint32_t memory_end = 0x1000 * 0x1000 * 0x10; // 16 MB

    num_frames = memory_end / PAGE_SIZE;
    frames = (Bitset)kmalloc(sizeof(bitset_t));
    bitset_create(frames, num_frames);

    kernel_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;

    // Identity mapping!
    uint32_t i;
    for (i = 0; i < kmalloc_first_unused; i += PAGE_SIZE) {
        alloc_frame(get_page(i, TRUE, kernel_directory), FALSE, FALSE);
    }

    printk("Enabling paging... ");
    isr_register_handler(PAGE_FAULT_ISR, page_fault_handler);
    switch_page_directory(kernel_directory);
    printk("Done\n");
}

page_t* get_page(uint32_t address, int create_missing, page_directory_t *dir) {
    address /= PAGE_SIZE;
    const uint32_t table_index = address / PAGES_PER_TABLE;
    const uint32_t page_index  = address % PAGES_PER_TABLE;
    if (dir->tables[table_index]) {
        return &dir->tables[table_index]->pages[page_index];
    } else if (create_missing) {
        uint32_t physical;

        dir->tables[table_index] = \
            (page_table_t*)kmalloc_ap(sizeof(page_table_t), &physical);
        memset(dir->tables[table_index], 0, sizeof(page_table_t));

        dir->tables_physical_addr[table_index] = \
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

void page_fault_handler(registers_t regs) {
    uint32_t fault_address;
    asm volatile ("movl %%cr2, %0" : "=r"(fault_address));

    int present =   (regs.err_code & 1);
    int on_write =  (regs.err_code & 2);
    int user_mode = (regs.err_code & 4);
    int reserved =  (regs.err_code & 8);
    int op_fetch =  (regs.err_code & 16);

    printk("Page-fault: page %spresent; failed on %s in %s-mode at 0x%08x\n",
        present     ? ""        : "NOT "    ,
        on_write    ? "write"   : "read"    ,
        user_mode   ? "user"    : "kernel"  ,
        fault_address
    );
    if (reserved) printk("Also trampled reserved bits\n");
    if (op_fetch) printk("Error occurred during op-fetch\n");

    panic("Page fault!");
}
