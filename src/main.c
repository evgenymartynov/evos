#include "monitor.h"

struct multiboot_t;

int kmain(struct multiboot_t *mboot) {
    monitor_clear();
    int i;
    for (i = 0; i < 80; i++)
        monitor_put(1);
    monitor_write("Hello, cruel world... :(");
    monitor_write("?? O_o ");
    monitor_write_hex(0x0DEFACED);
    monitor_write_dec(1234567890);
    monitor_write_dec(123);
    monitor_write_dec(120030);
    monitor_write_dec(0);
    monitor_write_dec(-123);

    return 0x00DEFACED;
}
