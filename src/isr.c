#include "isr.h"

isr_handler_t interrupt_handlers[256];

isr_handler_t isr_register_handler(uint32_t int_no, isr_handler_t handler) {
    return interrupt_handlers[int_no] = handler;
}
