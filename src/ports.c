#include "ports.h"

void outb(port_t port, uint8_t value) {
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(port_t port) {
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a" (value) : "dN" (port));
    return value;
}

uint16_t inw(port_t port) {
    uint16_t value;
    asm volatile ("inw %1, %0" : "=a" (value) : "dN" (port));
    return value;
}
