all:
	make -C src all
	scripts/update-image.sh
	make run

clean:
	make -C src clean

run:
	bochs -f bochs.rc

.PHONY: all clean run
