#ifndef _STRING_H_
#define _STRING_H_

#include "stdint.h"

void memset(void* ptr, uint8_t value, uint32_t size);
int is_digit(char c);
int strlen(const char *string);

#endif