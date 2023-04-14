#!/bin/bash

# shell folder
current_folder=$(cd "$(dirname "$0")" || exit;pwd)

# Linux 64
#export PATH="/home/cn1396/.toolchain/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu/bin/:$PATH"

# For cross compile
#export ARCH=arm64
#export CROSS_COMPILE=aarch64-none-linux-gnu-

rm -rf ${current_folder}/out
mkdir -p ${current_folder}/out

# create binary
aarch64-none-linux-gnu-gcc -g -static -o out/hello hello/hello.c
aarch64-none-linux-gnu-gcc -g -o out/ctest crypto/ctest.c \
    crypto/crypto_data.c crypto/crypto_util.c \
    crypto/crypto_hash_test.c
aarch64-none-linux-gnu-gcc -g -static -o out/block_device block_device/block_device.c

# dump asm
#aarch64-none-linux-gnu-objdump -xd hello/hello >hello/hello.asm

