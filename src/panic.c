#include "panic.h"
#include "printk.h"
#include "stdint.h"
#include "string.h"

typedef struct {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;
} __attribute__((packed)) multiboot_elf_section_header_t;

typedef struct {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    uint8_t  info;
    uint8_t  other;
    uint16_t shndx;
} __attribute__((packed)) elf_symbol_t;

#define ELF_SYMBOL_TYPE(x)      ((x) & 0xf)
#define ELF_SYMBOL_FUNCTION     2

static elf_symbol_t *symbols;
static const char *strtab;
static uint32_t num_symbols   = 0;
static uint32_t strtab_length = 0;

void init_panic_backtrace(multiboot_info_t *mboot) {
    if (!(mboot->flags & (1 << 5))) {
        printk("multiboot did not provide ELF info");
        report_fail();
        return;
    }

    printk("Parsing multiboot ELF data");

    multiboot_elf_section_header_t *sh = \
        (multiboot_elf_section_header_t*)mboot->u.elf_sec.addr;
    uint32_t shstrtab = sh[mboot->u.elf_sec.shndx].addr;

    int i;
    for (i = 0; i < mboot->u.elf_sec.num; i++) {
        const char *name = (const char *)(shstrtab + sh[i].name);

        if (strcmp(name, ".strtab") == 0) {
            strtab = (const char*)sh[i].addr;
            strtab_length = sh[i].size;
        }

        if (strcmp(name, ".symtab") == 0) {
            symbols = (elf_symbol_t*)sh[i].addr;
            num_symbols = sh[i].size / sizeof(elf_symbol_t);
        }
    }

    report_success();
}

static const char* backtrace_lookup(uint32_t addr) {
    int i;
    for (i = 0; i < num_symbols; i++) {
        if (ELF_SYMBOL_TYPE(symbols[i].info) != ELF_SYMBOL_FUNCTION) {
            continue;
        }

        uint32_t sym_start = symbols[i].value;              // Inclusive
        uint32_t sym_end   = sym_start + symbols[i].size;   // Exclusive
        if (sym_start <= addr && addr < sym_end) {
            return (const char*)(strtab + symbols[i].name);
        }
    }

    return "(null)";
}

static void print_stack_trace(void) {
    uint32_t *ebp, *eip;
    asm volatile ("movl %%ebp, %0" : "=r"(ebp));

    while (ebp) {
        eip = ebp + 1;
        printk("  [%#08x] %s\n", *eip, backtrace_lookup(*eip));
        ebp = (uint32_t*)*ebp;
    }
}

void __panic(const char *file, const char *func, int line, const char *msg) {
    printk("\n\rKernel panic. Stack trace:\n");
    print_stack_trace();

    printk("'%s' in file %s:%d: ", func, file, line);
    printk(msg);

    asm volatile ("cli; hlt");
    for(;;);
}
