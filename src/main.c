#include "monitor.h"

struct multiboot_t;

int kmain(struct multiboot_t *mboot) {
    monitor_clear();
    int i;
    for (i = 0; i < 80; i++)
        monitor_put(1);
    for (i = 0; i < 10; i++) {
        monitor_write_dec(i*8);
        monitor_put('\t');
    }
    monitor_write("Hello, cruel world... :(\n");
    monitor_write("?? O_o you don't see me");
    monitor_write("\rThis is on the same line!\n");
    monitor_write("And\t this is tabbled!\t\t!\n");
    monitor_write_hex(0x0DEFACED);
    monitor_write_dec(1234567890);
    monitor_write_dec(123);
    monitor_write_dec(120030);
    monitor_write_dec(0);
    monitor_write_dec(123);
    monitor_put(0x08);
    monitor_put('4');
    monitor_put('\n');

    for (i = 0; i < 20; i++) {
        monitor_write_dec(i);
        monitor_put('\n');
    }

    monitor_write("Reached the end of kmain()\n");
    return 0x00DEFACED;
}
