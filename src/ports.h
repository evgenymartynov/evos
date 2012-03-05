#ifndef _PORTS_H_
#define _PORTS_H_

#include "stddef.h"

typedef uint16_t port_t;

void     outb(port_t port, uint8_t value);
uint8_t  inb(port_t port);
uint16_t inw(port_t port);

#endif