#ifndef _PANIC_H_
#define _PANIC_H_

#include "multiboot.h"

void __panic(const char *file, const char *func, int line, const char *msg);
#define panic(msg...) __panic(__FILE__, __func__, __LINE__, msg)

void init_panic_backtrace(multiboot_info_t *mboot);

#endif
