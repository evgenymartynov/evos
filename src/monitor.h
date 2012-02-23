#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "ports.h"

void monitor_put(char c);
void monitor_clear();
void monitor_write(char *str);
void monitor_write_hex(uint32_t value);
void monitor_write_dec(uint32_t value);
void monitor_write_status(const char *str, int success);

#endif