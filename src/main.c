#include "monitor.h"

struct multiboot_t;

int kmain(struct multiboot_t *mboot) {
    int i;
    for (i = 0; i < 80; i++)
        monitor_put(1);
    return 0x00DEFACED;
}
