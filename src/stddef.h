#ifndef _STDDEF_H_
#define _STDDEF_H_

#define FALSE 0
#define TRUE  1

#define BUILD_BUG_UNLESS(x) (sizeof(struct { int _: !!(x); } ))
#define BUILD_BUG_ON_SIZEOF(type, size)                 \
    static int __check_##type __attribute__((unused)) = \
    BUILD_BUG_UNLESS(sizeof(type) == size);

#endif
