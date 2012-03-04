#include "printf.h"
#include "stddef.h"
#include "string.h"
#include "usermode/syscall.h"

static char buf[4096];

uint32_t printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int length = vsprintf(fmt, buf, args);
    va_end(args);

    int num_written = write(buf, length);
    return num_written;
}
