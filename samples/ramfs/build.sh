#!/bin/bash

# shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

# Linux 64
export PATH="/home/cn1396/.toolchain/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu/bin/:$PATH"

# For cross compile
export ARCH=arm64
export CROSS_COMPILE=aarch64-none-linux-gnu-

rm -rf image

# make /dev/console for printf
mkdir -p image/dev
sudo mknod -m 660 image/dev/console c 5 1 

# create init
aarch64-none-linux-gnu-gcc -g -static -o image/init hello.c


aarch64-none-linux-gnu-objdump -xd image/init >image/init.asm

#echo hello | cpio -o --format=newc > initramfs
