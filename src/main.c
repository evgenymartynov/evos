#include "monitor.h"
#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "paging.h"
#include "panic.h"
#include "stdint.h"
#include "multiboot.h"
#include "mem.h"
#include "kmalloc.h"

static void __attribute__((unused)) test_screen(void) {
    int i;
    for (i = 0; i < 80; i++)
        monitor_put(1);
    for (i = 0; i < 10; i++) {
        monitor_write_dec(i*8);
        monitor_put('\t');
    }
    monitor_write("Hello, cruel world... :(\n");
    monitor_write("?? O_o you don't see me");
    monitor_write("\rThis is on the same line! ");
    monitor_write("And\tthis is tabbled!\t\t!\n");

    for (i = 1; i <= 10; i++) {
        printk("%d", i);
        if (i == 10) printk("!\n");
        else printk(", ");
    }

    printk("Testing printk() formats: %b, %o, %d, %x\n", 15, 15, 15, 15);
    printk("Testing printk() widths: %b, %4b, %4b\n", 8, 8, 5);
    printk("Testing printk() padding: %d, %5d, %5d, %4d, %3d\n", 123, 123, -123, -123, -123);
    printk("Testing printk() align: %5d, %-5d, %-5d, %-4d, %-3d\n", 123, 123, -123, -123, -123);
    printk("Testing printk() prefixes: %5x, %-5x, %#5x, %#-5x\n", 123, 123, 123, 123);
    printk("Testing signs\n 123: %d\n4bil: %u\n-123: %d\n", 123, -123, -123);
    printk("Testing string widths\n");
    const char *s = "abcdefgh";
    printk("%s %5s %-10s %10s\n", s, s, s, s, s);
}

static void __attribute__((unused)) test_kmalloc(void) {
    printk("Testing heap allocation and freeing");

    uint32_t a = kmalloc(4);
    uint32_t b = kmalloc(4);
    uint32_t c = kmalloc(4);
    uint32_t first_alloc = a;

    kfree((void*)c); c = 0;
    c = kmalloc(16);

    kfree((void*)a); a = 0;
    kfree((void*)b); b = 0;
    a = kmalloc(8);

    kfree((void*)a); a = 0;
    kfree((void*)b); b = 0;
    kfree((void*)c); c = 0;

    a = kmalloc(32);
    if (a != first_alloc) {
        report_fail();
    } else {
        report_success();
    }
}

// This is the entry point from boot.s
// We need to change the stack ASAP
// Interrupts are disabled at this point in time
int kmain(multiboot_info_t *mboot) {
    monitor_clear();
    init_panic_backtrace(mboot);
    init_mem(mboot);
    init_gdt();
    init_idt();
    init_paging(); // Also enables kernel heap

    // Switch to a different stack. GRUB leaves us in an undefined state.
    #define STACK_SZ 0x100000
    char *new_stack = (char*)kmalloc_a(STACK_SZ) + STACK_SZ;
    char *old_stack = 0;
    asm volatile (
        "movl %%esp, %0 \n"
        "movl %1, %%esp \n"
        "call __kmain   \n"
        "movl %0, %%esp \n"
        : "=m"(old_stack)
        : "m"(new_stack)
    );
    #undef STACK_SZ

    monitor_write("Reached the end of kmain()\n");
    return 0x00DEFACED;
}

// This is a continuation of kmain(...), with a new stack
// Interrupts are still disabled here.
void __kmain(void) {
    init_timer(50);

    asm volatile ("sti");

    printk("Testing interrupts: ");
    asm volatile ("int $0x3");

    // test_screen();
    test_kmalloc();
}
