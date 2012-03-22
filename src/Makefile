LDFLAGS=-Tlink.ld
ASFLAGS=-felf
CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Werror -I. 

# Optimise code without breaking it... hopefully
CFLAGS+=-O3 -fno-optimize-sibling-calls

SOURCES=boot.o main.o ports.o monitor.o printk.o string.o 	\
		gdt.o gdt_asm.o idt.o idt_asm.o isr.o panic.o 		\
		timer.o kmalloc.o kheap.o

SOURCES += mm/mem.o mm/frame.o mm/paging.o mm/page_manip.o
SOURCES += datastructs/bitset.o datastructs/ordered_array.o

SOURCES += syscall.o
SOURCES += usermode/syscall.o usermode/printf.o

.DEFAULT_GOAL=all
.PHONY=all clean

all: $(SOURCES)
	ld $(LDFLAGS) -o ../bin/kernel $(SOURCES)

clean:
	-find . -name '*.o' -exec rm {} \;
	-rm ../bin/kernel

.s.o:
	nasm $(ASFLAGS) $<
