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
#include "syscall.h"

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

char *new_stack;

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
    new_stack = (char*)kmalloc_a(STACK_SZ) + STACK_SZ;
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

void umode(void) {
    uint32_t written = printf("User-mode!\n");
    printf("That write() said it wrote %d characters\n", written);
    for(;;);
}

void jump_usermode(void) {
    printk("Transitioning into ring 3\n\n");

    asm volatile(
        "movw $0x23, %%ax;"
        "movw %%ax, %%ds;"
        "movw %%ax, %%es;"
        "movw %%ax, %%fs;"
        "movw %%ax, %%gs;"

        "movl %%esp, %%eax;"
        "pushl $0x23;"
        "pushl %%eax;"
        "pushf;"
        "pop %%eax; or $0x200, %%eax; push %%eax;"
        "pushl $0x1b;"
        "pushl $umode;"
        "iret;"
        : : : "eax"
    );
}

// This is a continuation of kmain(...), with a new stack
// Interrupts are still disabled here.
void __kmain(void) {
    init_timer(50);
    init_syscalls();

    asm volatile ("sti");

    printk("Testing interrupts: ");
    asm volatile ("int $0x3");

    test_kmalloc();

    jump_usermode();
}
