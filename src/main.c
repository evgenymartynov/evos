#include "monitor.h"
#include "printk.h"
#include "gdt.h"

struct multiboot_t;

int kmain(struct multiboot_t *mboot) {
    monitor_clear();
    init_gdt();

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
    monitor_write_hex(0x0DEFACED);
    monitor_write_dec(1234567890);
    monitor_put(0x08);
    monitor_put('4');
    monitor_put('\n');

    for (i = 1; i <= 10; i++) {
        printk("%d", i);
        if (i == 10) printk("!\n");
        else printk(", ");
    }

    printk("Testing signs\n 123: %d\n4bil: %u\n-123: %d\n", 123, -123, -123);

    monitor_write("Reached the end of kmain()\n");
    return 0x00DEFACED;
}
