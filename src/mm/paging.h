#ifndef _PAGING_H_
#define _PAGING_H_

#include "stddef.h"
#include "isr.h"

#define PAGES_PER_TABLE         1024
#define TABLES_PER_DIRECTORY    1024

typedef struct {
    uint32_t  present       :1;
    uint32_t  writeable     :1;
    uint32_t  user_mode     :1;
    uint32_t  __reserved_1  :2;
    uint32_t  accessed      :1;
    uint32_t  is_dirty      :1;
    uint32_t  __reserved_2  :2;
    uint32_t  __unused      :3;
    uint32_t address        :20;
} __attribute__((packed)) page_t;

typedef struct {
    page_t pages[PAGES_PER_TABLE];
} __attribute__((packed)) page_table_t;

typedef struct {
    // Actual tables in the virtual addressing scheme
    page_table_t    *tables[TABLES_PER_DIRECTORY];
    // Physical locations of the above.
    uint32_t tables_physical_addr[TABLES_PER_DIRECTORY];
    // Physical location of the above. Addresception.
    uint32_t ismeta_tables_physical_addr;
} page_directory_t;

void init_paging(void);
void switch_page_directory(page_directory_t *dir);
page_directory_t *clone_directory(page_directory_t *src);

void page_alloc(page_t *page, int kernel_mode, int writeable);
void page_free(page_t *page);
page_t* get_page(uint32_t address, int create_missing, page_directory_t *dir);

void page_fault_handler(registers_t *regs);

#endif
