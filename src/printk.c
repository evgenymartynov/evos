#include "printk.h"
#include "stddef.h"
#include "monitor.h"
#include "string.h"

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
#define LEFT_ALIGN  4
#define SHOW_PREFIX 8

static char* append_number(char *str, unsigned int value, int radix, int flags, int width) {
    #define BUFSIZE 64

    // This should not overrun with 32 bit parameters
    char _buf[BUFSIZE + 1] = {};
    char *buf = _buf;
    char sign = 0;

    if (radix < 2 || radix > 16) {
        return str;
    }

    if (width > BUFSIZE) {
        width = BUFSIZE;
    }

    const char *digits = "0123456789abcdef";
    const char padding = (flags & ZERO_PADDED)
                         ? '0'
                         : ' ';

    if (flags & SIGNED) {
        if ((int)value < 0) {
            sign = '-';
            value = (-(int)value);
        }
    }

    // NOTE: Populate buf[] backwards
    unsigned int i = value;
    do {
        unsigned int digit = i % radix;
        *buf++ = digits[digit];

        i /= radix;
        width--;
    } while (i != 0 || (!(flags & LEFT_ALIGN) && width > 0));

    if (flags & SHOW_PREFIX) {
        if (radix == 2) {
            *buf++ = 'b';
            *buf++ = '0';
        } else if (radix == 8) {
            *buf++ = '0';
        } else if (radix == 16) {
            *buf++ = 'x';
            *buf++ = '0';
        }
    }

    if (sign) {
        *buf++ = sign;
    }

    if (!(flags & LEFT_ALIGN)) {
        for (; width > 0; width--) {
            *buf++ = padding;
        }
    }

    do {
        buf--;
        *str++ = *buf;
    } while (buf != _buf);

    if (flags & LEFT_ALIGN) {
        for (; width > 0; width--) {
            *str++ = padding;
        }
    }

    return str;
    #undef BUFSIZE
}

static const char* skip_atoi(const char *str, int *target) {
    *target = 0;

    while (is_digit(*str)) {
        *target *= 10;
        *target += *str - '0';
        str++;
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

        // Parse flags
        int flags = 0;
        int flags_done = FALSE;
        do {
            fmt++;

            switch (*fmt) {
                case '-': flags |= LEFT_ALIGN;  break;
                case '0': flags |= ZERO_PADDED; break;
                case '#': flags |= SHOW_PREFIX; break;
                default:
                    flags_done = TRUE;
            }
        } while (!flags_done);

        // Get field width
        int width = 0;
        if (is_digit(*fmt)) {
            fmt = skip_atoi(fmt, &width);
        }

        // Determine the type and print it
        switch (*fmt) {
            case '%':
                *str++ = *fmt;
                break;

            case 'd':
                flags |= SIGNED;
            case 'u':
                str = append_number(str, va_arg(args, int), 10, flags, width);
                break;

            case 'x':
                str = append_number(str, va_arg(args, int), 16, flags, width);
                break;

            case 'o':
                str = append_number(str, va_arg(args, int), 8, flags, width);
                break;

            case 'b':
                str = append_number(str, va_arg(args, int), 2, flags, width);
                break;

            case 's':
                {
                    char *param = va_arg(args, char*);
                    int len = strlen(param);
                    int i;

                    if (!(flags & LEFT_ALIGN)) {
                        while (len < width) {
                            *str++ = ' ';
                            width--;
                        }
                    }

                    for (i = 0; i < len; i++) {
                        *str++ = *param++;
                    }

                    while (len < width) {
                        *str++ = ' ';
                        width--;
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

void report_success(void) {
    monitor_write_status("DONE", 1);
}

void report_fail(void) {
    monitor_write_status("FAIL", 0);
}
