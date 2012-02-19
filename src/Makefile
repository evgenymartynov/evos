LDFLAGS=-Tlink.ld
ASFLAGS=-felf
CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Werror -I.

SOURCES=boot.o main.o ports.o monitor.o printk.o string.o 	\
		gdt.o gdt_asm.o idt.o idt_asm.o

.DEFAULT_GOAL=all

all: $(SOURCES) link

clean:
	-rm *.o ../bin/kernel

link:
	ld $(LDFLAGS) -o ../bin/kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
