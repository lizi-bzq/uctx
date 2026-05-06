#!/bin/bash
for arch in x86_64 aarch64 arm mips mipsel i386; do
    make distclean && make ARCH=$arch || exit 1
    case $arch in
        x86_64)  timeout 30 ./test ;;
        aarch64) timeout 30 qemu-aarch64 -L /usr/aarch64-linux-gnu ./test ;;
        arm)     timeout 30 qemu-arm -L /usr/arm-linux-gnueabi ./test ;;
        mips)    timeout 30 qemu-mips -L /usr/mips-linux-gnu ./test ;;
        mipsel)  timeout 30 qemu-mipsel -L /usr/mipsel-linux-gnu ./test ;;
        i386)    timeout 30 qemu-i386 ./test ;;
    esac
done
