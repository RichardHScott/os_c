#include "paging.h"

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

const struct page_table* p4_table = 0xfffffffffffff000;

static int64_t read_tlb() {
    int64_t val;
    asm volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

static void write_tlb(int64_t val) {
    asm volatile ("mov %0, %%cr3" : : "r"(val));
}

static void flush_tlb() {
    write_tlb(read_tlb());
}

static void invalidate_page(intptr_t addr) {
    asm volatile("invlpg (%0)"
                :
                : "r"(addr)
                : "memory");
}

void set_unused(struct page_table_entry* entry) {
    entry->entry = 0x0;
}

void init_page_table(struct page_table* table) {
    for(int i=0; i<PAGE_TABLE_ENTRY_COUNT; ++i) {
        set_unused(&table->entries[i]);
    }
}

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
    assert(((!physical_addr_mask) & get_frame_start_addr(frame)) == 0);

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

void get_page_for_vaddr(virtual_addr_t vaddr, struct page* p) {
    assert(vaddr < 0x0000800000000000 || vaddr >= 0xffff800000000000);

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

int translate_page(struct page *page, struct frame *frame) {
    struct page_table* table = p4_table;

    table = descened_page_table(table, get_p4_index(page));

    if(table->entries[get_p3_index(page)].entry & huge_mask) {
        //todo
    }

    table = descened_page_table(table, get_p3_index(page));

    if(table->entries[get_p2_index(page)].entry & huge_mask) {
        //todo 
        
        if(0 != get_physical_frame(&table->entries[get_p2_index(page)], frame))
            return -1;

        frame->number += get_p1_index(page);

        return 0;
    }

    table = descened_page_table(table, get_p2_index(page));

    return get_physical_frame(&table->entries[get_p1_index(page)], frame);
}

physical_addr_t translate(virtual_addr_t vaddr) {
     struct page page;
     struct frame f;
     get_page_for_vaddr(vaddr, &page);
     translate_page(&page, &f);

     return (f.number * PAGE_SIZE) + (vaddr % PAGE_SIZE);
}

struct page_table* get_next_page_table_or_create(struct page_table* table, uint16_t index) {
    struct page_table* tbl = descened_page_table(table, index);

    if(tbl == NULL) {
        struct frame frame;
        int success = allocate_frame(&frame);
        
        if(success == 0) {
            set_page_table_entry(&table->entries[index], &frame, present_mask | writeable_mask);
            flush_tlb();
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

void identity_map_page(struct frame *frame, uintptr_t flags) {
    struct page p;
    get_page_for_vaddr(get_frame_start_addr(frame), &p);
    map_page_to_frame(&p, frame, flags);
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

    flush_tlb();
}

static intptr_t get_p4_table_phys_addr() {
    return read_tlb();
}

void remap_kernel() {
    struct page page;
    page.number = 0xdeadbeef;
    struct frame new_p4_frame;
    int success = allocate_frame(&new_p4_frame);
    terminal_printf("Allocation %x\n", success);
    map_page_to_frame(&page, &new_p4_frame, present_mask | writeable_mask);
    terminal_printf("Mapped frame\n");

    struct page old_p4_page;
    old_p4_page.number = 0xdddd0000;
    struct frame old_p4_frame;
    get_frame_for_addr(&old_p4_frame, (uintptr_t)read_tlb());
    terminal_printf("Old page addr: %#zx\n", old_p4_frame.number);
    map_page_to_frame(&old_p4_page, &old_p4_frame, present_mask | writeable_mask);    
    flush_tlb();

    struct page_table *old_p4_table = old_p4_page.number * PAGE_SIZE;
    init_page_table(page.number*PAGE_SIZE);

    terminal_printf("CR3 = %#zx\n", read_tlb());
    terminal_printf("Old p4 entry[511] %#zX\n", p4_table->entries[511]);
    terminal_printf("Old p4 entry[511] %#zX\n", old_p4_table->entries[511]);
    terminal_printf("New p4 frame addr %#zX\n", get_frame_start_addr(&new_p4_frame));

    set_page_table_entry(page.number*PAGE_SIZE + (8 * 511), &new_p4_frame, present_mask | writeable_mask);

    set_page_table_entry(&p4_table->entries[511], &new_p4_frame, present_mask | writeable_mask);
    terminal_printf("new p4 entry[511] %#zX\n", p4_table->entries[511]);
    flush_tlb();

    struct multiboot_elf_section_header* section;
    for(int i=0; i<data.elf_symbols->num; ++i) {
        section = &data.elf_symbols->sectionheaders[i];

        if(!elf_section_is_allocated(section)) {
            continue;
        }

        struct frame frame;
        uintptr_t addr = section->sh_addr;
        uintptr_t end_addr = section->sh_addr + section->sh_size - 1;

        terminal_printf("Addr: %#zx\n", addr);
        assert(addr % PAGE_SIZE == 0);
        assert(addr <= end_addr);

        while(addr < end_addr) {
            get_frame_for_addr(&frame, addr);
            addr += PAGE_SIZE;
            identity_map_page(&frame, writeable_mask | present_mask);
        }
    }

    //id map the vga buffer
    struct frame vga_frame;
    get_frame_for_addr(&vga_frame, 0xb8000);
    identity_map_page(&vga_frame, writeable_mask | present_mask);

    //id map the multiboot structure
    for(intptr_t mboot_addr = (intptr_t)data.start % PAGE_SIZE; mboot_addr <= data.start + data.start->total_size; mboot_addr += PAGE_SIZE) {
        struct frame mboot_frame;
        get_frame_for_addr(&mboot_frame, mboot_addr);
        identity_map_page(&mboot_frame, present_mask | writeable_mask);
    }

    terminal_printf("New kernel page tables set up.\n");

    //restore old p4
    set_page_table_entry(&old_p4_table->entries[511], &old_p4_frame, present_mask | writeable_mask);
    flush_tlb();
    terminal_printf("Old p4 entry[511] %#zX\n", p4_table->entries[511]);

    unmap_page(&old_p4_page);

    write_tlb(new_p4_frame.number*PAGE_SIZE | present_mask | writeable_mask);
    flush_tlb();
    
    //create guard page from old p4 frame
    struct page guard_page;
    guard_page.number = old_p4_frame.number;
    unmap_page(&guard_page);

    terminal_printf("End of remap.\n");
}
