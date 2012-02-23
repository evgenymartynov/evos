#include "panic.h"
#include "printk.h"

void __panic(const char *file, const char *func, int line, const char *msg) {
    printk("\n\rKernel panic!\n"
        "'%s' in file %s:%d: ",
        func, file, line);
    printk(msg);

    asm volatile ("cli; hlt");
    for(;;);
}
