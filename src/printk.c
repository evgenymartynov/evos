#include "printk.h"
#include "stddef.h"
#include "monitor.h"
#include "string.h"

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
