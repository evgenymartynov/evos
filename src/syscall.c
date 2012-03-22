#include "syscall.h"
#include "isr.h"
#include "stddef.h"
#include <task/task.h>
#include "printk.h"
static uint32_t tty_write(const char *data, uint32_t count) {
    int i;
    for (i = 0; i < count; i++) {
        printk("%c", data[i]);
    }

    return count;
}

static uint32_t null_syscall() {
    printk("Null syscall\n");
    return -1;
}

#define NUM_SYSCALLS 2
#define SYSCALL(nr, handler) [nr] = &handler
static void *syscalls[NUM_SYSCALLS] = {
    SYSCALL(0, null_syscall),
    SYSCALL(1, tty_write),
};
#undef SYSCALL

static void syscall_handler(registers_t *regs) {
    // printk("Received syscall %#x\n", regs->eax);
    if (regs->eax >= NUM_SYSCALLS) {
        printk("Invalid syscall! Returning.\n");
        regs->eax = -1;
        return;
    }

    void *func = syscalls[regs->eax];
    asm volatile (
        "pushl %5;"
        "pushl %4;"
        "pushl %3;"
        "pushl %2;"
        "pushl %1;"
        "call *%6;"
        "movl %%eax, %0;"
        "add $20, %%esp;"
        : "=m"(regs->eax)
        : "m"(regs->ebx), "m"(regs->ecx), "m"(regs->edx), "m"(regs->esi), "m"(regs->edi),  "r"(func)
    );
}

void init_syscalls(void) {
    isr_register_handler(ISR_SYSCALL, syscall_handler);
}
