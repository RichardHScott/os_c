#pragma once

#include <stdint.h>
#include "multiboot.h"

const size_t page_size = 4096;

struct frame {
    size_t number;
};

// Chunk frames into 4096 byte areas and indexed linearly

struct frame_allocator {
    struct frame next_free_frame;
    struct multiboot_memory_map *mem_map;
    size_t entry_index;

    uintptr_t kernel_start;
    uintptr_t kernel_end;

    uintptr_t multiboot_start;
    uintptr_t multiboot_end;
};

struct frame_allocator allocator;


void init_allocator(struct multiboot_data *data, int inital_index) {
    allocator.next_free_frame.number = 0;

    allocator.mem_map = data->memory_map;
    allocator.entry_index = inital_index;

    allocator.kernel_start = UINTPTR_MAX;
    allocator.kernel_end = 0;

    for(int i=0; i<data->elf_symbols->num; ++i) {
        struct multiboot_elf_section_header header = data->elf_symbols->sectionheaders[i];
        if(header.sh_size>0 && header.sh_addr<allocator.kernel_start) {
            allocator.kernel_start = header.sh_addr;
        }

        uintptr_t end = header.sh_size + header.sh_addr;
        if(end > allocator.kernel_end) {
            allocator.kernel_end = end;
        }
    }

    allocator.multiboot_start = start;
    allocator.multiboot_end = (uintptr_t)start + start->total_size;

    print_text("kernel start: ");
    print_hex_uint64(allocator.kernel_start);
    print_text(" kernel end: ");
    print_hex_uint64(allocator.kernel_end);
    print_newline();

    print_text("multiboot start: ");
    print_hex_uint64(allocator.multiboot_start);
    print_text(" multiboot end: ");
    print_hex_uint64(allocator.multiboot_end);
    print_newline();
}

void get_frame_for_addr(struct frame *frame, uintptr_t addr) {
    frame->number = addr / page_size;
}

void allocate_frame(struct frame *frame) {
}

void deallocate_frame(struct frame *frame) {

}
