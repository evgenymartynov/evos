#include "syscall.h"

uint32_t write(const void *data, uint32_t count) {
    int ret;
    asm volatile (
        "movl $1, %%eax;"
        "movl %2, %%ecx;"
        "movl %1, %%ebx;"
        "int $0x80;"
        "movl %%eax, %0;"
        : "=r"(ret)
        : "r" (data), "r"(count)
        : "ebx", "eax"
    );
    return ret;
}
