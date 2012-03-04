#ifndef _ISR_H_
#define _ISR_H_

#include "stdint.h"

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) registers_t;

typedef void (*isr_handler_t)(registers_t*);

isr_handler_t isr_register_handler(uint32_t int_no, isr_handler_t handler);

extern isr_handler_t interrupt_handlers[256];

#define PAGE_FAULT_ISR  14
#define ISR_SYSCALL     0x80

#endif
