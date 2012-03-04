#include "gdt.h"
#include "stdint.h"
#include "stddef.h"
#include "printk.h"
#include "string.h"

typedef struct {
    uint8_t segment_params  :3;
    uint8_t is_code         :1;
    uint8_t __reserved      :1;
    uint8_t ring            :2;
    uint8_t present         :1;
} __attribute__((packed)) access_flags_t;

typedef struct {
    uint8_t limit_high      :4; // Last 4 bits of the addressable-size limit
    uint8_t __reserved      :2;
    uint8_t operand_size    :1;
    uint8_t granularity     :1;
} __attribute__((packed)) granularity_flags_t;

struct gdt_entry {
    uint16_t limit_lower;   // Lower word of the addressable-size limit
    uint16_t base_lower;    // Lower word of the base
    uint8_t  base_mid;      // Mid byte of the base
    access_flags_t access_flags;        // GDT access flags
    granularity_flags_t granularity;    // GDT granularity + flags
    uint8_t  base_high;     // High byte of the base
} __attribute__((packed));

typedef struct {
    uint16_t limit; // Upper word of addressable-size limit
    uint32_t base;  // Address of the first GDT entry
} __attribute__((packed)) gdt_ptr_t;

typedef struct {
   uint32_t prev_tss;   // Pointer to previous TSS entry; NULL for us
   uint32_t esp0;   // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;    // The stack segment to load when we change to kernel mode.
   uint32_t esp1;
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;
   uint32_t cs;
   uint32_t ss;
   uint32_t ds;
   uint32_t fs;
   uint32_t gs;
   uint32_t ldt;
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

BUILD_BUG_ON_SIZEOF(access_flags_t, 1);
BUILD_BUG_ON_SIZEOF(granularity_flags_t, 1);
BUILD_BUG_ON_SIZEOF(gdt_entry_t, 8);
BUILD_BUG_ON_SIZEOF(gdt_ptr_t, 6);

//
// Actual GDT stuffs
//

#define GDT_SEG_ACCESSED            1   // Not sure what the hell that is
#define GDT_SEG_CODE_READABLE       2   // Read permission
#define GDT_SEG_CODE_CONFORMING     4   // If segment respects the ring model
#define GDT_SEG_DATA_WRITEABLE      2   // Write permission
#define GDT_SEG_DATA_DIRECTION      4   // Which direction the segment grows

#define NUM_GDT_ENTRIES 6
gdt_entry_t gdt_entries[NUM_GDT_ENTRIES];
gdt_ptr_t   gdt_ptr;
tss_entry_t tss;

extern void gdt_load(uint32_t);
static void gdt_set_entry(gdt_entry_t *ent, uint32_t base, uint32_t limit, access_flags_t access_flags, int use_4kib);
static void write_tss(gdt_entry_t *ent);

void init_gdt(void) {
    gdt_ptr.limit = sizeof(gdt_entry_t)*NUM_GDT_ENTRIES - 1; // yes, -1 is right
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    access_flags_t af = {};

    // Null segment
    gdt_set_entry(&gdt_entries[0], 0, 0, af, 0);

    // Code segment
    af.present = 1;
    af.ring = 0;
    af.__reserved = 1;  // Stupid Intel architects >_>
    af.is_code = 1;
    af.segment_params = GDT_SEG_CODE_READABLE;
    gdt_set_entry(&gdt_entries[1], 0, 0xFFFFFFFF, af, 1);

    af.is_code = 0;
    gdt_set_entry(&gdt_entries[2], 0, 0xFFFFFFFF, af, 1);

    af.ring = 3;
    af.is_code = 1;
    gdt_set_entry(&gdt_entries[3], 0, 0xFFFFFFFF, af, 1);

    af.is_code = 0;
    gdt_set_entry(&gdt_entries[4], 0, 0xFFFFFFFF, af, 1);

    // TSS
    write_tss(&gdt_entries[5]);

    printk("Loading a new GDT");
    gdt_load((uint32_t)&gdt_ptr);
    report_success();

    // Flush the TSS
    asm volatile (
        "movl $0x2b, %%eax;"
        "ltr %%ax;"
        : : : "eax"
    );
}

static void write_tss(gdt_entry_t *ent) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss);

    ent->base_lower = base & 0xFFFF;
    ent->base_mid   = (base >> 16) & 0xFF;
    ent->base_high  = (base >> 24) & 0xFF;

    ent->access_flags.segment_params = 1;
    ent->access_flags.is_code = 1;
    ent->access_flags.__reserved = 0;
    ent->access_flags.ring = 3;
    ent->access_flags.present = 1;

    ent->granularity.limit_high  = (limit >> 16) & 0x0F;
    ent->limit_lower = limit & 0xFFFF;

    ent->granularity.__reserved = 0;
    ent->granularity.operand_size = 0;
    ent->granularity.granularity = 0;

    tss.ss0 = 0x10;
    extern char *new_stack;
    tss.esp0 = (uint32_t)new_stack;   // TODO: per-task ESP0
    tss.cs = 0x08 | 3; // can switch to it from ring 3
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10 | 3;
}

static void gdt_set_entry(gdt_entry_t *ent, uint32_t base, uint32_t limit, access_flags_t access_flags, int use_4kib) {

    // Address base
    ent->base_lower = base & 0xFFFF;
    ent->base_mid   = (base >> 16) & 0xFF;
    ent->base_high  = (base >> 24) & 0xFF;

    // Addressable limit
    if (limit > 0xFFFF) {
        // TODO: check that (limit & 0xFFFF) == 0
        limit >>= 16;
        use_4kib = 1;
    }
    ent->limit_lower = limit & 0xFFFF;
    ent->granularity.limit_high = (limit >> 16) & 0x0F;

    // Whether we use 4KiB granularity
    ent->granularity.granularity = use_4kib;

    // Whether we use 32-bit addressing
    ent->granularity.operand_size = 1;

    // Access flags
    ent->access_flags = access_flags;
}
