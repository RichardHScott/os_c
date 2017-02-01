#include "kmalloc.h"

const intptr_t heap_start_addr = 4096 * 512 * 512;
const intptr_t heap_size = 100 * 1024;

static intptr_t heap_addr;

static intptr_t align_addr(intptr_t addr, size_t alignment)  {
    return (alignment) ? ((addr+alignment-1) & ~(alignment-1)) : (addr);
}

struct free_node {
    struct free_node *next;
    size_t free_size;
} __attribute__ ((packed));

struct alloc_node {
    intptr_t unused;
    size_t size;
} __attribute__ ((packed));

const size_t minimum_alloc_size = sizeof(struct free_node) + 4*sizeof(uintptr_t);

static struct free_node *free_list_head;

void init_heap(void) {
    heap_addr = heap_start_addr;
    for(virtual_addr_t addr = heap_start_addr; addr < heap_start_addr + heap_size; addr += PAGE_SIZE) {
        struct page page;
        get_page_for_vaddr(addr, &page);
        map_page(&page, present_mask | writeable_mask);
    }

    free_list_head = (struct free_node*) heap_addr;
    free_list_head->next = NULL;
    free_list_head->free_size = heap_size;
}

struct alloc_node* allocate_memory(size_t required_bytes) {
    struct alloc_node *alloc_node = NULL;
    struct free_node *node = free_list_head;

    while(node->next != NULL) {
        if(node->next->free_size <= required_bytes) {
            struct free_node *old_node = node->next;
            struct free_node *new_next_node = node->next;

            size_t new_free_bytes = old_node->next->free_size - required_bytes;
            if(new_free_bytes < minimum_alloc_size) {
                required_bytes = old_node->next->free_size;
                new_next_node = old_node->next;
                new_free_bytes = 0;
            }

            if(new_free_bytes) {
                new_next_node = (intptr_t) old_node + sizeof(old_node) + required_bytes;
                new_next_node->free_size = new_free_bytes - sizeof(new_next_node);
                new_next_node->next = old_node->next;
            }

            alloc_node = (struct alloc_node*) old_node;
            alloc_node->unused = NULL;
            alloc_node->size = required_bytes;

            node->next = new_next_node;
            break;
        }

        node = node->next;
    }

    if(!alloc_node) {
        struct free_node *old_head = free_list_head;
        struct free_node *new_head = (intptr_t) old_head + sizeof(old_head) + required_bytes;
        
        new_head->free_size = old_head->free_size - required_bytes;
        new_head->next = old_head->next;

        alloc_node = (struct alloc_node*) old_head;
        alloc_node->unused = NULL;
        alloc_node->size = required_bytes;

        free_list_head = new_head;
    }

    return alloc_node;
}

intptr_t kmalloc(size_t bytes) {
    size_t alignment = 0;
    intptr_t new_addr = align_addr(heap_addr, alignment);
    intptr_t new_head_addr = new_addr + bytes;

    if(new_head_addr > (heap_start_addr + heap_size)) {
        return NULL;
    }

    heap_addr = new_head_addr;

    return new_addr;
}

intptr_t krealloc(intptr_t old_ptr, size_t bytes) {

}

void kfree(intptr_t addr) {

}