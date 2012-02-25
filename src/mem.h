#ifndef _MEM_H_
#define _MEM_H_

#include "multiboot.h"
#include "stdint.h"

extern uint32_t mem_first_unused;
extern uint32_t mem_total_bytes;

void init_mem(multiboot_info_t *mboot);

#endif
