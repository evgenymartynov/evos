all:
	make -C src all

clean:
	make -C src clean

image:
	scripts/update-image.sh

run: image
	bochs -f bochs.rc

runq: image
	qemu -fda floppy.img

.PHONY: all clean image run
.DEFAULT_GOAL=all
