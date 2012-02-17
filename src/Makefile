LDFLAGS=-Tlink.ld
ASFLAGS=-felf
CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -I.

SOURCES=boot.o main.o ports.o monitor.o

all: $(SOURCES) link

clean:
	-rm *.o ../bin/kernel

link:
	ld $(LDFLAGS) -o ../bin/kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
