#include "monitor.h"
#include "printk.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "panic.h"
#include "stddef.h"
#include "multiboot.h"
#include <mm/mem.h>
#include "kmalloc.h"
#include "syscall.h"
#include <task/task.h>

#include "usermode/printf.h"

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

char *kernel_relocated_stack;

// This is the entry point from boot.s
// We need to change the stack ASAP
// Interrupts are disabled at this point in time
int kmain(multiboot_info_t *mboot) {
    monitor_clear();
    // Extract ELF symbols for nice panic stack traces
    init_panic_backtrace(mboot);
    // Initialise frame-allocator, paging, heap
    init_mem(mboot);

    // Switch to a different stack. GRUB leaves us in an undefined state.
    #define STACK_SZ 0x10000
    kernel_relocated_stack = (char*)kmalloc_a(STACK_SZ) + STACK_SZ;
    char *old_stack = 0;
    asm volatile (
        "movl %%esp, %0 \n"
        "movl %1, %%esp \n"
        "call __kmain   \n"
        "movl %0, %%esp \n"
        : "=m"(old_stack)
        : "m"(kernel_relocated_stack)
    );
    #undef STACK_SZ

    monitor_write("Reached the end of kmain()\n");
    return 0x00DEFACED;
}

void umode(void) {
    asm volatile(
        ".loop:"
            "movl $1, %%eax;"
            "movl %0, %%ebx;"
            "movl %1, %%ecx;"
            "int $0x80;"
            ".here: jmp .here;"
            "jmp .loop;"
        :
        : "r" ("1234\n"), "r"(5)
        : "eax", "ebx", "ecx"
    );
}

void umode2(void) {
    asm volatile(
        ".loop2:"
            "movl $1, %%eax;"
            "movl %0, %%ebx;"
            "movl %1, %%ecx;"
            "int $0x80;"
            "jmp .loop2;"
        :
        : "r" ("5678\n"), "r"(5)
        : "eax", "ebx", "ecx"
    );
}

// This is a continuation of kmain(...), with a new stack
// Interrupts are still disabled here.
void __kmain(void) {
    init_gdt(1);
    init_idt();
    init_timer(50);
    init_syscalls();

    asm volatile ("sti");

    printk("Testing interrupts: ");
    asm volatile ("int $0x3");

    test_kmalloc();

    init_tasking();

    // TODO: actually use init()
    pid_t init_pid = task_create(umode, 0);
    printk("Spawned a task with pid=%d\n", init_pid);
    init_pid = task_create(umode2, 0);
    printk("Spawned a task with pid=%d\n", init_pid);
}
