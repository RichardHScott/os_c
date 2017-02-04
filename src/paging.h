#pragma once
#include <stdint.h>
#include "frame_allocator.h"
#include "terminal.h"
#include "assert.h"
#include <stdbool.h>
#include "multiboot.h"

enum paging_masks {
    present_bit = 0x1,
    writeable_bit = 0x2,
    user_access_bit = 0x4,
    write_through_cache_bit = 0x8,
    disable_cache_bit = 0x10,
    accessed_bit = 0x20,
    dirty_bit = 0x40,
    huge_bit = 0x80,
    global_bit = 0x100,
    available_mask = 0xe,
    physical_addr_mask = 0x000ffffffffff000,
    os_defined_mask = 0x7ff0000000000000,
    no_exec_bit = 0x8000000000000000
};

#define PAGE_TABLE_ENTRY_COUNT 512
#define RECURSIVE_PAGE_TABLE_INDEX 511

typedef uintptr_t virtual_addr_t;
typedef uintptr_t physical_addr_t;

struct page {
    size_t number;
};

struct page_table_entry {
    uintptr_t entry;
};

struct page_table {
    struct page_table_entry entries[PAGE_TABLE_ENTRY_COUNT];
} __attribute__((packed));

void remap_kernel(void);

void get_page_for_vaddr(virtual_addr_t vaddr, struct page* p);
void map_page(struct page *page, uintptr_t flags);
void unmap_page(struct page *page);