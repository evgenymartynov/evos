#ifndef _STRING_H_
#define _STRING_H_

#include "stddef.h"
#include "stddef.h"

void memset(void* ptr, uint8_t value, uint32_t size);
void memcpy(void *dest, void *src, uint32_t num);
int is_digit(char c);
int strlen(const char *string);
int strcmp(const char *a, const char *b);
int vsprintf(const char *fmt, char *buf, va_list args);

#endif
