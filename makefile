CC=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/i686-elf-gcc-6.3.0
CCX=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/i686-elf-g++
AS=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/i686-elf-as

CFLAGS=-std=gnu99 -ffreestanding -Wall -Wextra
CCXFLAGS=

objects=kernel.o boot.o terminal.o

all: myos.bin

myos.bin: $(objects)
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o terminal.o -lgcc

boot.o: boot.s
	$(AS) boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

terminal.o: terminal.c
	$(CC) $(CFLAGS) -c terminal.c -o terminal.o

clean:
	rm myos.bin $(objects)