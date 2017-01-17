CC=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/$(arch)-elf-gcc-6.3.0
CCX=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/$(arch)-elf-g++
AS=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/$(arch)-elf-as
LD=/mnt/c/Users/Linuva/Dropbox/c/cross-chain/bin/$(arch)-elf-ld

CFLAGS=-std=gnu99 -ffreestanding -Wall -Wextra
CCXFLAGS=

arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
target ?= $(arch)-unknown-none-gnu

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg

assembly_source_files := $(wildcard src/arch/$(arch)/*.s)
assembly_object_files := $(patsubst src/arch/$(arch)/%.s, build/arch/$(arch)/%.o, $(assembly_source_files))

c_source_files := $(wildcard src/*.c)
c_object_files := $(patsubst src/%.c, build/%.o, $(c_source_files))

all: $(kernel)

$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	$(LD) -n -o $(kernel) -T $(linker_script) $(assembly_object_files) $(c_object_files)

build/arch/$(arch)/%.o: src/arch/$(arch)/%.s
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@
#	$(AS) --64 -ad $< -o $@

build/%.o: src/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -r build

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles