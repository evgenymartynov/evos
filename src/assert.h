#ifndef _ASSERT_H_
#define _ASSERT_H_

#include "panic.h"

#define assert(condition, message)                                      \
    do {                                                                \
        if (!(condition)) {                                             \
            panic("\n  Assertion failed: " #condition "\n  " message);  \
        }                                                               \
    } while (0)

#endif
