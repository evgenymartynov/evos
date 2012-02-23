#ifndef _KMALLOC_H_
#define _KMALLOC_H_

#include "stdint.h"

uint32_t kmalloc(uint32_t size);
uint32_t kmalloc_a(uint32_t size);
uint32_t kmalloc_p(uint32_t size, uint32_t *physical);
uint32_t kmalloc_ap(uint32_t size, uint32_t *physical);

#endif
