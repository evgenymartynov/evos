#include "printk.h"
#include "stddef.h"
#include "monitor.h"

typedef char* va_list;

#define __va_stacksize(arg)     \
    ( ( ( sizeof(arg) + sizeof(int)-1 ) / sizeof(int) ) * sizeof(int) )

#define va_start(va, prev_arg)  \
    ( va = (char*)&prev_arg + __va_stacksize(prev_arg) )
#define va_arg(va, type)        \
    ( va += __va_stacksize(type),  *(type*)(va - __va_stacksize(type)) )
#define va_end(va)

#define SIGNED 1

static char* append_number(char *str, unsigned int value, int flags) {
    if (flags & SIGNED) {
        if (value & (1 << (sizeof(value)*8-1))) {
            *str++ = '-';
            value = (-(int)value);
        }
    }

    int i;
    int had_digit = FALSE;

    for (i = 1000000000; i >= 1; i /= 10) {
        unsigned int digit = (value / i) % 10;

        if (digit == 0 && !had_digit && i != 1)
            continue;

        *str++ = ('0' + digit);
        had_digit = TRUE;
    }

    return str;
}

static int vsprintf(const char *fmt, char *buf, va_list args) {
    char *str;

    for (str = buf; *fmt; fmt++) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }

        // Consume the '%'
        fmt++;

        int flags = 0;

        // TODO: process flags

        switch (*fmt) {
            case '%':
                *str++ = *fmt;
                break;

            case 'd':
                flags |= SIGNED;
            case 'u':
                str = append_number(str, va_arg(args, int), flags);
                break;

            // TODO: more switches

            default:
                *str++ = *fmt;
        }
    }

    *str++ = '\0';
    return str - buf;
}

static char buf[4096];

// TODO: fix up buffer overruns
int printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int num_written = 0;
    vsprintf(fmt, buf, args);
    monitor_write(buf);

    va_end(args);
    return num_written;
}
