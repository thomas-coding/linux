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
aarch64-none-linux-gnu-gcc -g -static -o out/hello hello/hello.c || exit
aarch64-none-linux-gnu-gcc -g -o out/ctest crypto/ctest.c \
    crypto/crypto_data.c crypto/crypto_util.c \
    crypto/crypto_hash_test.c || exit
aarch64-none-linux-gnu-gcc -g -static -o out/block_device block_device/block_device.c || exit

aarch64-none-linux-gnu-gcc -g -static -o out/udp_server net/udp_server.c || exit
aarch64-none-linux-gnu-gcc -g -static -o out/udp_client net/udp_client.c || exit
aarch64-none-linux-gnu-gcc -g -static -o out/tcp_server net/tcp_server.c || exit
aarch64-none-linux-gnu-gcc -g -static -o out/tcp_client net/tcp_client.c || exit

aarch64-none-linux-gnu-gcc -g -static -o out/ucontext ucontext/ucontext.c || exit
aarch64-none-linux-gnu-gcc -g -static -o out/debuggee ptrace/debuggee.c || exit
aarch64-none-linux-gnu-gcc -g -static -o out/debugger ptrace/debugger.c || exit

aarch64-none-linux-gnu-gcc -g -static -o out/display display/display.c || exit

# dump asm
aarch64-none-linux-gnu-objdump -xd out/debuggee > out/debuggee.asm

# build host
rm -rf ${current_folder}/host/out
mkdir -p ${current_folder}/host/out
gcc -g -static -o host/out/tcp_server  host/net_test/tcp_server.c  || exit
gcc -g -static -o host/out/tcp_client  host/net_test/tcp_client.c  || exit
