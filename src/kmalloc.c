#include "kmalloc.h"

const intptr_t heap_start_addr = 4096 * 512 * 512;
const intptr_t heap_size = 100 * 1024;

static intptr_t heap_addr;

static intptr_t align_addr(intptr_t addr, size_t alignment)  {
    return (alignment) ? ((addr+alignment-1) & ~(alignment-1)) : (addr);
}

struct free_node {
    struct free_node *next;
    size_t size;
} __attribute__ ((packed));

const size_t minimum_alloc_size = sizeof(struct free_node) + 2*sizeof(uintptr_t);

static struct free_node *free_list_head;

static void compact_heap() {
    struct free_node *node = free_list_head;

    while(node != NULL) {
        

        node = node->next;
    }
}

void init_heap(void) {
    heap_addr = heap_start_addr;
    for(virtual_addr_t addr = heap_start_addr; addr < heap_start_addr + heap_size; addr += PAGE_SIZE) {
        struct page page;
        get_page_for_vaddr(addr, &page);
        map_page(&page, present_mask | writeable_mask);
    }

    free_list_head = (struct free_node*) heap_addr;
    free_list_head->next = NULL;
    free_list_head->size = heap_size;
}

struct free_node* allocate_memory(size_t required_bytes) {
    struct free_node *alloc_node = NULL;
    struct free_node *node = free_list_head;

    terminal_printf("Required bytes %zx\n", required_bytes);
    while(node->next != NULL) {
        terminal_printf("Old size: %zx\n", node->next->size);

        if(node->next->size >= required_bytes) {
            terminal_printf("Not from head, old node: %zx\n", node->next);
            struct free_node *old_node = node->next;
            struct free_node *new_next_node = node->next->next;

            size_t new_free_bytes = old_node->size - required_bytes;
            if(new_free_bytes < minimum_alloc_size) {
                required_bytes = old_node->size;
                new_free_bytes = 0;
            }

            if(new_free_bytes) {
                terminal_printf("New free bytes: %zx\n", new_free_bytes);
                new_next_node = (intptr_t) old_node + sizeof(struct free_node) + required_bytes;
                terminal_printf("New next node: %zx\n", new_next_node);
                new_next_node->size = new_free_bytes - sizeof(struct free_node);
                new_next_node->next = old_node->next;
            }

            alloc_node = old_node;
            alloc_node->next = NULL;
            alloc_node->size = required_bytes;

            node->next = new_next_node;
            break;
        }

        node = node->next;
    }

    if(!alloc_node) {          
        terminal_printf("From head\n");
        struct free_node *old_head = free_list_head;
        struct free_node *new_head = (intptr_t) old_head + sizeof(struct free_node) + required_bytes;
        
        new_head->size = old_head->size - required_bytes - sizeof(struct free_node);
        new_head->next = old_head->next;

        alloc_node = old_head;
        alloc_node->next = NULL;
        alloc_node->size = required_bytes;

        free_list_head = new_head;
    }

    return alloc_node;
}

intptr_t kmalloc(size_t bytes) {
    size_t alignment = 8;

    size_t req_bytes = bytes;

    struct free_node *node = allocate_memory(req_bytes);

    return (intptr_t) node + sizeof(struct free_node);
}

intptr_t krealloc(intptr_t old_ptr, size_t bytes) {

}

void kfree(intptr_t addr) {
    struct free_node *node = (struct free_node*) (addr - sizeof(struct free_node));
    terminal_printf("Free node base %zx\n", (intptr_t) node);
    node->next = free_list_head->next;
    free_list_head->next = node;
}