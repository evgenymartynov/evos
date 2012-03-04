#ifndef _GDT_H_
#define _GDT_H_

typedef struct gdt_entry gdt_entry_t;
void init_gdt(int init_tss);

#endif
