#!/bin/bash

mkdir -p isodir/boot/grub
cp out/myos.bin isodir/boot/myos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o out/myos.iso isodir

