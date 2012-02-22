#include "idt.h"
#include "ports.h"
#include "isr.h"

#include "stdint.h"
#include "stddef.h"
#include "printk.h"
#include "string.h"

typedef struct {
    uint8_t gate_type       :4;
    uint8_t storage_segment :1;
    uint8_t privilege_level :2;
    uint8_t present         :1;
}__attribute__((packed)) idt_flags_t;

typedef struct {
    uint16_t    handler_addr_lower;
    uint16_t    segment;
    uint8_t     __reserved;
    idt_flags_t flags;
    uint16_t    handler_addr_upper;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t size;
    uint32_t entries_addr;
} __attribute__((packed)) idt_ptr_t;

BUILD_BUG_ON_SIZEOF(idt_flags_t, 1);
BUILD_BUG_ON_SIZEOF(idt_entry_t, 8);
BUILD_BUG_ON_SIZEOF(idt_ptr_t, 6);

static void test_idt_structs(void) {
    #define __cast(type, value) (*(type*) &value)
    #define check(type, af, value) (__cast(type, af) == (value))
    #define test(blob, value) if (!check(uint8_t, blob, value)) {             \
            printk("In %s: FAILED test on %s\n", __FILE__, #blob);            \
            printk("\tGot %o, expected %o\n", __cast(uint8_t, blob), value);  \
        }

    idt_flags_t flags;
        flags.gate_type = 0;
        flags.storage_segment = 0;
        flags.privilege_level = 0;
        flags.present = 0;
    test(flags, 0000);
        flags.gate_type = 9;
    test(flags, 0011);
        flags.storage_segment = 1;
    test(flags, 0031);
        flags.privilege_level = 3;
    test(flags, 0171);
        flags.present = 1;
    test(flags, 0371);

    #undef test
    #undef check
    #undef __cast
}

//
// Actual code below
//

#define IDT_32BIT_INTERRUPT_GATE 0x0E
#define PIC_MASTER_LOW_INT 32
#define PIC_SLAVE_LOW_INT 40

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

// Great. Just great.
extern void isr_0(void);
extern void isr_1(void);
extern void isr_2(void);
extern void isr_3(void);
extern void isr_4(void);
extern void isr_5(void);
extern void isr_6(void);
extern void isr_7(void);
extern void isr_8(void);
extern void isr_9(void);
extern void isr_10(void);
extern void isr_11(void);
extern void isr_12(void);
extern void isr_13(void);
extern void isr_14(void);
extern void isr_15(void);
extern void isr_16(void);
extern void isr_17(void);
extern void isr_18(void);
extern void isr_19(void);
extern void isr_20(void);
extern void isr_21(void);
extern void isr_22(void);
extern void isr_23(void);
extern void isr_24(void);
extern void isr_25(void);
extern void isr_26(void);
extern void isr_27(void);
extern void isr_28(void);
extern void isr_29(void);
extern void isr_30(void);
extern void isr_31(void);
extern void irq_0(void);
extern void irq_1(void);
extern void irq_2(void);
extern void irq_3(void);
extern void irq_4(void);
extern void irq_5(void);
extern void irq_6(void);
extern void irq_7(void);
extern void irq_8(void);
extern void irq_9(void);
extern void irq_10(void);
extern void irq_11(void);
extern void irq_12(void);
extern void irq_13(void);
extern void irq_14(void);
extern void irq_15(void);

static void idt_load(idt_ptr_t *new_idt) {
    __asm__("lidt %0" :: "m"(*new_idt));
}

static void idt_set_handler(idt_entry_t *ent, uint32_t address, uint16_t segment, idt_flags_t flags) {
    ent->handler_addr_lower = address & 0xFFFF;
    ent->handler_addr_upper = (address >> 16) & 0xFFFF;

    ent->segment = segment;
    ent->__reserved = 0;
    ent->flags = flags;
}

static void remap_pic(void) {
    // TODO: save masks?
    // Do not want
    outb(0x20, 0x11);   // 0x20 is master PIC; 0xA0 is slave PIC
    outb(0xA0, 0x11);

    outb(0x21, PIC_MASTER_LOW_INT);     // Set the interrupt vector offsets
    outb(0xA1, PIC_SLAVE_LOW_INT);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0x0);    // Reset masks to 0
    outb(0xA1, 0x0);
}

void init_idt(void) {
    test_idt_structs();

    idt_ptr.size = sizeof(idt_entries)-1;
    idt_ptr.entries_addr = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entries));

    #define SET_HANDLER(i, segment, flags)  \
        idt_set_handler(&idt_entries[i], (uint32_t)&isr_##i, segment, flags);

    idt_flags_t flags;
    flags.gate_type = IDT_32BIT_INTERRUPT_GATE;
    flags.storage_segment = 0;
    flags.privilege_level = 0;
    flags.present = 1;

    // I hate this
    SET_HANDLER(0,  0x08, flags);
    SET_HANDLER(1,  0x08, flags);
    SET_HANDLER(2,  0x08, flags);
    SET_HANDLER(3,  0x08, flags);
    SET_HANDLER(4,  0x08, flags);
    SET_HANDLER(5,  0x08, flags);
    SET_HANDLER(6,  0x08, flags);
    SET_HANDLER(7,  0x08, flags);
    SET_HANDLER(8,  0x08, flags);
    SET_HANDLER(9,  0x08, flags);
    SET_HANDLER(10, 0x08, flags);
    SET_HANDLER(11, 0x08, flags);
    SET_HANDLER(12, 0x08, flags);
    SET_HANDLER(13, 0x08, flags);
    SET_HANDLER(14, 0x08, flags);
    SET_HANDLER(15, 0x08, flags);
    SET_HANDLER(16, 0x08, flags);
    SET_HANDLER(17, 0x08, flags);
    SET_HANDLER(18, 0x08, flags);
    SET_HANDLER(19, 0x08, flags);
    SET_HANDLER(20, 0x08, flags);
    SET_HANDLER(21, 0x08, flags);
    SET_HANDLER(22, 0x08, flags);
    SET_HANDLER(23, 0x08, flags);
    SET_HANDLER(24, 0x08, flags);
    SET_HANDLER(25, 0x08, flags);
    SET_HANDLER(26, 0x08, flags);
    SET_HANDLER(27, 0x08, flags);
    SET_HANDLER(28, 0x08, flags);
    SET_HANDLER(29, 0x08, flags);
    SET_HANDLER(30, 0x08, flags);
    SET_HANDLER(31, 0x08, flags);
    #undef SET_HANDLER
    #define SET_IRQ_HANDLER(i, segment, flags)  \
        idt_set_handler(&idt_entries[32+i], (uint32_t)&irq_##i, segment, flags);
    SET_IRQ_HANDLER(0,  0x08, flags);
    SET_IRQ_HANDLER(1,  0x08, flags);
    SET_IRQ_HANDLER(2,  0x08, flags);
    SET_IRQ_HANDLER(3,  0x08, flags);
    SET_IRQ_HANDLER(4,  0x08, flags);
    SET_IRQ_HANDLER(5,  0x08, flags);
    SET_IRQ_HANDLER(6,  0x08, flags);
    SET_IRQ_HANDLER(7,  0x08, flags);
    SET_IRQ_HANDLER(8,  0x08, flags);
    SET_IRQ_HANDLER(9,  0x08, flags);
    SET_IRQ_HANDLER(10, 0x08, flags);
    SET_IRQ_HANDLER(11, 0x08, flags);
    SET_IRQ_HANDLER(12, 0x08, flags);
    SET_IRQ_HANDLER(13, 0x08, flags);
    SET_IRQ_HANDLER(14, 0x08, flags);
    SET_IRQ_HANDLER(15, 0x08, flags);
    #undef SET_IRQ_HANDLER

    printk("About to load a new IDT... ");
    idt_load(&idt_ptr);
    printk("Loaded\n");

    printk("Remapping the PIC... ");
    remap_pic();
    printk("Done\n");
}

// Called from assembly
void isr_common_handler(registers_t regs) {
    printk("Received interrupt %d\n", regs.int_no);

    if (regs.int_no == 13) {
        printk("General Protection Fault\n");
    }
}

// Called from assembly
void irq_common_handler(registers_t regs) {
    isr_handler_t handler = interrupt_handlers[regs.int_no];
    if (handler) {
        handler(regs);
    }

    // Notify slave PIC that we are done
    if (regs.int_no >= PIC_SLAVE_LOW_INT) {
        outb(0xA0, 0x20);
    }

    // And the master
    outb(0x20, 0x20);
}
