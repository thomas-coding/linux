#!/bin/bash

# shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# Linux 64
export PATH="/home/cn1396/.toolchain/riscv64-glibc-ubuntu-20.04-gcc-nightly-2024.04.12-nightly/bin:$PATH"
export CROSS_COMPILE=riscv64-unknown-linux-gnu-

#export PATH="/home/cn1396/.toolchain/nuclei_riscv_glibc_prebuilt_linux64_2022.04/bin:$PATH"
#export CROSS_COMPILE=riscv-nuclei-linux-gnu-

# For cross compile
export ARCH=riscv


rm -rf image

# make /dev/console for printf
mkdir -p image/dev
sudo mknod -m 660 image/dev/console c 5 1 

# for mmap
sudo mknod -m 660 image/dev/mem c 1 1
sudo chown root:kmem image/dev/mem

# create init
riscv64-unknown-linux-gnu-gcc -g -static -o image/init hello.c
#riscv-nuclei-linux-gnu-gcc -g -static -o image/init hello.c

cd image
rm -rf ../rootfs.cpio
find . | fakeroot cpio -o -H newc > ../rootfs.cpio
gzip -f ../rootfs.cpio

# update to out
rm -rf ../../../../out/thomas_riscv64/images/rootfs.cpio.gz
cp ../rootfs.cpio.gz ../../../../out/thomas_riscv64/images/


#riscv64-unknown-linux-gnu-objdump -xd image/init >image/init.asm

#echo hello | cpio -o --format=newc > initramfs