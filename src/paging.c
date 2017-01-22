#include "paging.h"

struct page {
    size_t number;
};

const uintptr_t present_mask = 0x1;
const uintptr_t writeable_mask = 0x2;
const uintptr_t user_access_mask = 0x4;
const uintptr_t write_through_cache_mask = 0x8;
const uintptr_t disable_cache_mask = 0x10;
const uintptr_t accessed_mask = 0x20;
const uintptr_t dirty_mask = 0x40;
const uintptr_t huge_mask = 0x80;
const uintptr_t global_mask = 0x100;
const uintptr_t available_mask = 0xe;
const uintptr_t physical_addr_mask = 0x000ffffffffff000;
const uintptr_t os_defined_mask = 0x7ff0000000000000;
const uintptr_t no_exec_mask = 0x8000000000000000;

struct page_table_entry {
    uintptr_t entry;
};

#define PAGE_TABLE_ENTRY_COUNT 512

struct page_table {
    struct page_table_entry entries[PAGE_TABLE_ENTRY_COUNT];
} __attribute__((packed));

void set_unused(struct page_table_entry* entry) {
    entry->entry = 0x0;
}

void init_page_table(struct page_table* table) {
    for(int i=0; i<PAGE_TABLE_ENTRY_COUNT; ++i) {
        set_unused(&table->entries[i]);
    }
}


const struct page_table* p4_table = 0xfffffffffffff000;

struct page_table* descened_page_table(struct page_table *table, size_t index) {
    if(index >= 512) {
        return NULL;
    }

    if(table->entries[index].entry & (present_mask | !huge_mask)) {
        return (struct page_table*)( ((uintptr_t)table->entries << 9) | (index << 12) );
    }

    return NULL;
}

void set_page_table_entry(struct page_table_entry *entry, struct frame *frame, uintptr_t flags) {
    //assert that the frame is aligned to 4096 bytes
    //assert(((!physical_addr_mask) & get_frame_start_addr(frame)) == 0);

    entry->entry = (get_frame_start_addr(frame) & physical_addr_mask) | flags;
}

/* Return 0 if success. 1 if frame is not present in memory.
 *
 */
int get_physical_frame(struct page_table_entry *entry, struct frame *f) {
    if(entry->entry & present_mask) {
        get_frame_for_addr(f, entry->entry & physical_addr_mask);
        return 0;
    }

    return -1;
}

typedef uintptr_t virtual_addr;
typedef uintptr_t physical_addr;

void get_page_for_vaddr(virtual_addr vaddr, struct page* p) {
    if(vaddr >= 0x0000800000000000 && vaddr < 0xffff800000000000 ) {
        //invalid page, bit 48 must be sign extended
    }

    p->number = vaddr / PAGE_SIZE;
}

uint16_t get_p4_index(struct page* p) {
    return (p->number >> 27) & 0777;
}

uint16_t get_p3_index(struct page* p) {
    return (p->number >> 18) & 0777;
}

uint16_t get_p2_index(struct page* p) {
    return (p->number >> 9) & 0777;
}

uint16_t get_p1_index(struct page* p) {
    return p->number & 0777;
}

void translate_page(struct page *page, struct frame *frame) {
    struct page_table* table = p4_table;

    table = descened_page_table(table, get_p4_index(page));

    if(table->entries[get_p3_index(page)].entry & huge_mask) {
        //todo
    }

    table = descened_page_table(table, get_p3_index(page));

    if(table->entries[get_p2_index(page)].entry & huge_mask) {
        //todo
        get_physical_frame(&table->entries[get_p2_index(page)], frame);
        frame->number += get_p1_index(page);

        return;
    }

    table = descened_page_table(table, get_p2_index(page));

    get_physical_frame(&table->entries[get_p1_index(page)], frame);
}

physical_addr translate(virtual_addr vaddr) {
     struct page page;
     struct frame f;
     get_page_for_vaddr(vaddr, &page);
     translate_page(&page, &f);

     return (f.number * PAGE_SIZE) + (vaddr % PAGE_SIZE);
}

struct page_table* get_next_page_table_or_create(struct page_table* table, uint8_t index) {

    struct page_table* tbl = descened_page_table(table, index);

    if(tbl == NULL) {
        struct frame frame;
        int success = allocate_frame(&frame);
        
        if(success == 0) {
            set_page_table_entry(&table->entries[index], &frame, present_mask | writeable_mask);
        }
    }

    return descened_page_table(table, index);
}

void map_page_to_frame(struct page *page, struct frame *frame, uintptr_t flags) {
    struct page_table* p3_table = get_next_page_table_or_create(p4_table, get_p4_index(page));
    struct page_table* p2_table = get_next_page_table_or_create(p3_table, get_p3_index(page));
    struct page_table* p1_table = get_next_page_table_or_create(p2_table, get_p2_index(page));

    //make sure unused here!

    set_page_table_entry(&p1_table->entries[get_p1_index(page)], frame, present_mask | flags);
}

void map_page(struct page *page, uintptr_t flags) {
    struct frame frame;
    int success = allocate_frame(&frame);
    map_page_to_frame(page, &frame, flags);
}

void unmap_page(struct page *page) {
    struct page_table* table = p4_table;

    table = descened_page_table(table, get_p4_index(page));
    table = descened_page_table(table, get_p3_index(page));
    table = descened_page_table(table, get_p2_index(page));

    set_unused(&table->entries[get_p1_index(page)]);
    
    struct frame frame;
    get_physical_frame(&table->entries[get_p1_index(page)], &frame);
    deallocate_frame(&frame);
}

void test(void) {
    print_text("Translation for 0x0 is: ");
    print_hex_uint64(translate(0));
    print_newline();


    print_text("Translation for 4096 is: ");
    print_hex_uint64(translate(4096));
    print_newline();

    print_text("Translation for (512 * 4096) is: ");
    print_hex_uint64(translate((512 * 4096)));
    print_newline();

    print_text("Translation for (300 * 512 * 4096) is: ");
    print_hex_uint64(translate(299 * 512 * 4096));
    print_newline();

    for(uintptr_t i=0; i<UINTPTR_MAX; ++i) {
        uintptr_t trans = translate(i);
        if(i != trans) {
            
            print_text("i: ");
            print_hex_uint64(i);

            print_text(" trans: ");
            print_hex_uint64(trans);
            break;
        }
    }
}