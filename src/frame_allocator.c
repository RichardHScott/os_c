#include "frame_allocator.h"

const size_t page_size = 4096;

void init_allocator(struct multiboot_data *data) {
    allocator.next_free_frame.number = 0;

    allocator.mem_map = data->memory_map;

    allocator.num_entries = ((uintptr_t)(allocator.mem_map->size) - 4*sizeof(uint32_t))/allocator.mem_map->entry_size;

    allocator.entry_index = 0;
    uint64_t min_addr = UINT64_MAX;
    for(int i=0; i<allocator.num_entries; ++i) {
        if(min_addr > allocator.mem_map->memory_maps[i].base_addr
            && allocator.mem_map->memory_maps[i].length >= page_size) {
            min_addr = allocator.mem_map->memory_maps[i].base_addr;
            allocator.entry_index = i;
        }
    }

    uintptr_t kernel_start_addr = UINTPTR_MAX;
    uintptr_t kernel_end_addr = 0;

    for(int i=0; i<data->elf_symbols->num; ++i) {
        struct multiboot_elf_section_header header = data->elf_symbols->sectionheaders[i];
        if(header.sh_size>0 && header.sh_addr<kernel_start_addr) {
            kernel_start_addr = header.sh_addr;
        }

        uintptr_t end = header.sh_size + header.sh_addr;
        if(end > kernel_end_addr) {
            kernel_end_addr = end;
        }
    }

    get_frame_for_addr(&allocator.kernel_start, kernel_start_addr);
    get_frame_for_addr(&allocator.kernel_end, kernel_end_addr);


    get_frame_for_addr(&allocator.multiboot_start, start);
    get_frame_for_addr(&allocator.multiboot_end, (uintptr_t)start + start->total_size);
}

void get_frame_for_addr(struct frame *frame, uintptr_t addr) {
    frame->number = addr / page_size;
}

static void switch_area() {
    uint64_t min_addr = allocator.mem_map->memory_maps[allocator.entry_index].base_addr + allocator.mem_map->memory_maps[allocator.entry_index].length - 1;

    uint64_t next_min_addr = UINT64_MAX;

    allocator.entry_index = allocator.num_entries;

    for(int i=0; i<allocator.num_entries; ++i) {
        if(min_addr <= allocator.mem_map->memory_maps[i].base_addr
            && allocator.mem_map->memory_maps[i].length >= page_size
            && next_min_addr > allocator.mem_map->memory_maps[i].base_addr) {
            next_min_addr = allocator.mem_map->memory_maps[i].base_addr;
            allocator.entry_index = i;
        }
    }
}

/* Returns 0 on success. Non-zero on failure.
 *
 */
int allocate_frame(struct frame *frame) {
    frame->number = 0;

    do {        
        if(allocator.entry_index == allocator.num_entries) {
            return -1;
        }

        size_t index = allocator.next_free_frame.number;

        struct frame last_frame_of_area;
        struct multiboot_memory_map_entry* entry = &allocator.mem_map->memory_maps[allocator.entry_index];
        get_frame_for_addr(&last_frame_of_area, entry->base_addr + entry->length - 1);

        if(index > last_frame_of_area.number) {
            //need to switch to the next allowed area
            switch_area();
        } else if(index >= allocator.kernel_start.number && index <= allocator.kernel_end.number) {
            ++allocator.next_free_frame.number;
        } else if(index >= allocator.multiboot_start.number && index <= allocator.multiboot_end.number) {
            ++allocator.next_free_frame.number;
        } else {
            frame->number = allocator.next_free_frame.number++;
            return 0;
        }
    } while(1);
}

void deallocate_frame(struct frame *frame) {

}
