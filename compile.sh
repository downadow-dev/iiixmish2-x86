#!/bin/bash
# [03/06/25] changes
# [03/07/25] iiixmish2-x86
# --downadow

set -e
set -v

gcc -o img-tool img-tool.c
./img-tool < "$1" > program.h

CFLAGS="-ffreestanding -fno-stack-protector -m32 -nostdinc -nostdlib -I. -O2 -Wall"
LDFLAGS="-Wl,-N -Wl,-T -Wl,debian/invaders.ld -Wl,--build-id=none"
gcc $CFLAGS -c keyboard.c
gcc $CFLAGS -c delay.c
gcc $CFLAGS -c common.c
gcc $CFLAGS -c main.c
gcc $CFLAGS -c char.c
gcc $CFLAGS -c kernel.c
gcc $CFLAGS -c video.c
gcc $CFLAGS -c memory.c
gcc $CFLAGS -c boot.S
gcc $CFLAGS $LDFLAGS -o os.exec boot.o kernel.o keyboard.o char.o video.o main.o delay.o common.o memory.o
objcopy -O binary os.exec os

