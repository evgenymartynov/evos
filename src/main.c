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

int kmain(multiboot_info_t *mboot) {
    monitor_clear();

    init_mem(mboot);
    init_gdt();
    init_idt();
    init_paging();
    init_timer(50);

    asm volatile ("sti");

    printk("Testing interrupts: ");
    asm volatile ("int $0x3");

    // test_screen();

    monitor_write("Reached the end of kmain()\n"); 
    return 0x00DEFACED;
}
