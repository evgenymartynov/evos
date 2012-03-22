#include "task.h"
#include "kheap.h"  // For kernel heap paging
#include "panic.h"
#include "string.h"
#include "mm/paging.h"
#include "printk.h"
#include "ports.h"

#define STACK_SIZE 4096*10
#define MAX_NUM_TASKS 256

#define TASK_IMAGE_BASE 0x10000000
#define TASK_KERNEL_STACK_BOTTOM 0x20000000
#define TASK_USER_STACK_BOTTOM   0xF0000000

static pid_t find_available_task(void);
static pid_t find_next_task(void);
static task_t tasks[MAX_NUM_TASKS];
static pid_t current_task;

void init_tasking(void) {
    current_task = -1;
}

pid_t task_create(void *image, uint32_t image_size) {
    pid_t pid = find_available_task();
    task_t *task = &tasks[pid];
    memset(task, 0, sizeof(*task));

    // Make it live!
    task->alive = TRUE;
    task->pid = pid;

    // Set up the registers
    memset(&task->saved_state, 0, sizeof(task->saved_state));
    task->saved_state.cs = 0x18 | 3;
    task->saved_state.eip = TASK_IMAGE_BASE;

    task->saved_state.ds = /*task->saved_state.es = task->saved_state.fs =*/ 0x20 | 3;
    task->saved_state.ss = task->saved_state.ds;
    task->saved_state.esp = TASK_USER_STACK_BOTTOM + 0x0FFC;

    // paging
    // TODO: set up the pages properly
    // TODO: page out a separate kernel stack for each task
    task->page_directory = clone_directory(current_directory);
    page_t *image_page = get_page(TASK_IMAGE_BASE, TRUE, task->page_directory);
    page_alloc(image_page, FALSE, TRUE);
    page_alloc(get_page(TASK_USER_STACK_BOTTOM, TRUE, task->page_directory), FALSE, TRUE);
    page_alloc(get_page(TASK_KERNEL_STACK_BOTTOM, TRUE, task->page_directory), FALSE, TRUE);

    // copy the image across
    mm_memcpy_physical(image_page->address * 0x1000, image, 0x1000);

    // TODO: fake stack with a return address that calls an exit() syscall

    return pid;
}

void task_switch(void) {
    pid_t next_task = find_next_task();

    task_t *task = &tasks[next_task];

    /*
        Alright, so.
        Set up a fake bunch of data on the task's stack.
        We then move our %esp to that, fix registers, segments, change %cr3, and do an iret.
        What can possibly go wrong?
    */

    current_task = next_task;

    uint32_t offset = task->saved_state.esp - TASK_USER_STACK_BOTTOM - 8*5;
    mm_memcpy_physical(
        get_page(TASK_USER_STACK_BOTTOM, FALSE, task->page_directory)->address * 0x1000 + offset,
        &task->saved_state.edi, 8*4
    );

    outb(0x20, 0x20);
    outb(0xA0, 0x20);

    asm volatile(
        "movl %0, %%eax;"
        "movl %%eax, %%cr3;"
        "movl %1, %%esp;"
        "pushl $0x23;"
        "pushl %1;"
        "sti;"
        "pushf;"
        // "pop %%eax; or $0x200, %%eax; push %%eax;"
        "pushl $0x1b;"
        "pushl %2;"
        // "pushl %%eax;"
        "movl $0x23, %%eax;"
        "movw %%ax, %%ds;"
        "movw %%ax, %%es;"
        "movw %%ax, %%fs;"
        "movw %%ax, %%gs;"
        // "popl %%eax;"
        "sub $32, %%esp;"
        "popa;"
        "iret;"
        :
        : "m"(task->page_directory->ismeta_tables_physical_addr), "r"(task->saved_state.esp), "m"(task->saved_state.eip)
        : "eax"
    );
}

static pid_t find_available_task(void) {
    int i;
    for (i = 0; i < MAX_NUM_TASKS; i++) {
        if (!tasks[i].alive) {
            return i;
        }
    }

    panic("No tasks available");
    return -1;
}

static pid_t find_next_task(void) {
    // TODO: stub
    pid_t next = current_task+1;
    int i;
    for (i = 0; i < MAX_NUM_TASKS; i++, next = (next+1) % MAX_NUM_TASKS) {
        if (tasks[next].alive) {
            return next;
        }
    }
    return -1;
}
