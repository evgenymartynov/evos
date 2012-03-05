#ifndef _STDDEF_H_
#define _STDDEF_H_

#define FALSE 0
#define TRUE  1

/* Compile-time asserts */
#define BUILD_BUG_UNLESS(x) (sizeof(struct { int _: !!(x); } ))
#define BUILD_BUG_ON_SIZEOF(type, size)                 \
    static int __check_##type __attribute__((unused)) = \
    BUILD_BUG_UNLESS(sizeof(type) == size);

/* Variable argument list defines */
typedef char* va_list;

#define __va_stacksize(arg)     \
    ( ( ( sizeof(arg) + sizeof(int)-1 ) / sizeof(int) ) * sizeof(int) )
#define va_start(va, prev_arg)  \
    ( va = (char*)&prev_arg + __va_stacksize(prev_arg) )
#define va_arg(va, type)        \
    ( va += __va_stacksize(type),  *(type*)(va - __va_stacksize(type)) )
#define va_end(va)

/* Standard integers */
typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;
typedef          int         int32_t;

#endif
