#include "timer.h"
#include "isr.h"
#include "ports.h"

#include "printk.h"

#define PIT_CLOCK_FREQ 1193180
#define IRQ_0 32

static void timer_callback(registers_t regs) {
    // printk("tick ");
}

void init_timer(uint32_t freq) {
    isr_register_handler(IRQ_0, timer_callback);

    uint32_t divisor = PIT_CLOCK_FREQ / freq;

    if (divisor & ~0xFFFF) {
        printk("WARN: frequency is too low\n");
        divisor = 0xFFFF;
    }

    printk("Initialising timer");
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor>>8) & 0xFF);
    report_success();
}