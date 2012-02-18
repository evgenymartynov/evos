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

#define SIGNED      1
#define ZERO_PADDED 2

static char* append_number(char *str, unsigned int value, int radix, int flags) {
    if (flags & SIGNED) {
        if (value & (1 << (sizeof(value)*8-1))) {
            *str++ = '-';
            value = (-(int)value);
        }
    }

    const char *digits = "0123456789abcdef";
    char _buf[20] = {}; // TODO: fix overruns of this
    char *buf = _buf;

    unsigned int i = value;
    do {
        unsigned int digit = i % radix;
        *buf++ = digits[digit];
        i /= radix;
    } while (i != 0 || ((flags & ZERO_PADDED) && buf-_buf < 8) );

    do {
        buf--;
        *str++ = *buf;
    } while (buf != _buf);

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
                str = append_number(str, va_arg(args, int), 10, flags);
                break;

            case 'x':
                *str++ = '0';
                *str++ = 'x';
                str = append_number(str, va_arg(args, int), 16, flags);
                break;

            case 'o':
                *str++ = '0';
                *str++ = 'o';
                str = append_number(str, va_arg(args, int), 8, flags);
                break;

            case 'b':
                *str++ = '0';
                *str++ = 'b';
                flags |= ZERO_PADDED;
                str = append_number(str, va_arg(args, int), 2, flags);
                break;

            case 's':
                {
                    char *param = va_arg(args, char*);
                    while (*param) {
                        *str++ = *param++;
                    }
                }
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
