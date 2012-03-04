#ifndef _STDDEF_H_
#define _STDDEF_H_

#define FALSE 0
#define TRUE  1

#define BUILD_BUG_UNLESS(x) (sizeof(struct { int _: !!(x); } ))
#define BUILD_BUG_ON_SIZEOF(type, size)                 \
    static int __check_##type __attribute__((unused)) = \
    BUILD_BUG_UNLESS(sizeof(type) == size);


typedef char* va_list;

#define __va_stacksize(arg)     \
    ( ( ( sizeof(arg) + sizeof(int)-1 ) / sizeof(int) ) * sizeof(int) )
#define va_start(va, prev_arg)  \
    ( va = (char*)&prev_arg + __va_stacksize(prev_arg) )
#define va_arg(va, type)        \
    ( va += __va_stacksize(type),  *(type*)(va - __va_stacksize(type)) )
#define va_end(va)

#endif
