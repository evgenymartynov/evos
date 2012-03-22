#ifndef _TASK_TASK_H_
#define _TASK_TASK_H_

#include "stddef.h"
#include "gdt.h"    // For tss_entry_t
#include <mm/paging.h>

typedef uint32_t pid_t;

typedef struct {
    pid_t pid;
    int alive; // Whether or not this is an actual task (and not a free task)

    page_directory_t *page_directory;
    registers_t saved_state;
} task_t;

void init_tasking(void);

pid_t task_create(void *image, uint32_t image_size);
void task_switch(void);
void task_exit(pid_t pid);

#endif
