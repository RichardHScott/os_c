#pragma once

#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

#define PAGE_SIZE 4096

struct frame {
    size_t number;
};

// Chunk frames into 4096 byte areas and indexed linearly

struct frame_allocator {
    struct frame next_free_frame;
    struct multiboot_memory_map *mem_map;
    size_t entry_index;
    size_t num_entries;

    struct frame kernel_start;
    struct frame kernel_end;

    struct frame multiboot_start;
    struct frame multiboot_end;
};

struct frame_allocator allocator;


void init_allocator(struct multiboot_data *data);

void get_frame_for_addr(struct frame *frame, uintptr_t addr);
uintptr_t get_frame_start_addr(struct frame *frame);

/* Returns 0 on success. Non-zero on failure.
 *
 */
int allocate_frame(struct frame *frame);

void deallocate_frame(struct frame *frame);
