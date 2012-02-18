all:
	make -C src all

clean:
	make -C src clean

image:
	scripts/update-image.sh

run: image
	bochs -f bochs.rc

.PHONY: all clean image run
