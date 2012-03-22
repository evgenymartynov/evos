#ifndef _GDT_H_
#define _GDT_H_

#include "stddef.h"

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

typedef struct gdt_entry gdt_entry_t;
void init_gdt(int init_tss);

#endif
