#!/bin/bash

set -e

echo "Generating image..."

sudo losetup /dev/loop0 floppy.img
sudo mount /dev/loop0 /mnt
sudo cp bin/kernel /mnt/kernel
sudo umount /dev/loop0
sudo losetup -d /dev/loop0

echo "Image generated successfully"
