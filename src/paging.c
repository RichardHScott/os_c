#include "paging.h"

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

static void enable_no_exec() {
    asm volatile("push %%rcx\n"
                 "mov $0xC0000080, %%ecx\n"
                 "rdmsr\n"
                 "or $0x800, %%eax\n"
                 "wrmsr\n"
                 "pop %%rcx"
                :
                :
                : "%ecx");
}

enum cr0_bits {
    cr0_write_protect = 1 << 16
};

static int64_t read_cr0() {
    int64_t val;
    asm volatile ("mov %%cr0, %0" : "=r"(val));
    return val;
}

static void write_cr0(int64_t val) {
    asm volatile ("mov %0, %%cr0" : : "r"(val));
}

static enable_write_protect() {
    write_cr0(read_cr0() | cr0_write_protect);
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

    if(table->entries[index].entry & (present_bit | !huge_bit)) {
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
    if(entry->entry & present_bit) {
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

    if(table->entries[get_p3_index(page)].entry & huge_bit) {
        //todo
    }

    table = descened_page_table(table, get_p3_index(page));

    if(table->entries[get_p2_index(page)].entry & huge_bit) {
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
            set_page_table_entry(&table->entries[index], &frame, present_bit | writeable_bit);
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

    set_page_table_entry(&p1_table->entries[get_p1_index(page)], frame, present_bit | flags);
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
    enable_no_exec();
    enable_write_protect();

    struct page page;
    page.number = 0xdeadbeef;
    struct frame new_p4_frame;
    int success = allocate_frame(&new_p4_frame);

    map_page_to_frame(&page, &new_p4_frame, present_bit | writeable_bit);

    struct page old_p4_page;
    old_p4_page.number = 0xdddd0000;
    struct frame old_p4_frame;
    get_frame_for_addr(&old_p4_frame, (uintptr_t)read_tlb());
    terminal_printf("Old page addr: %#zx\n", old_p4_frame.number);
    map_page_to_frame(&old_p4_page, &old_p4_frame, present_bit | writeable_bit);    
    flush_tlb();

    struct page_table *old_p4_table = old_p4_page.number * PAGE_SIZE;
    init_page_table(page.number*PAGE_SIZE);

    terminal_printf("CR3 = %#zx\n", read_tlb());
    terminal_printf("Old p4 entry[511] %#zX\n", p4_table->entries[511]);
    terminal_printf("Old p4 entry[511] %#zX\n", old_p4_table->entries[511]);
    terminal_printf("New p4 frame addr %#zX\n", get_frame_start_addr(&new_p4_frame));

    set_page_table_entry(page.number*PAGE_SIZE + (8 * 511), &new_p4_frame, present_bit | writeable_bit);

    set_page_table_entry(&p4_table->entries[511], &new_p4_frame, present_bit | writeable_bit);
    terminal_printf("new p4 entry[511] %#zX\n", p4_table->entries[511]);
    flush_tlb();

    for(int i=0; i<data.elf_symbols->num; ++i) {
        struct multiboot_elf_section_header* section = &data.elf_symbols->sectionheaders[i];

        if(!elf_section_is_allocated(section)) {
            continue;
        }

        uintptr_t addr = section->sh_addr;
        uintptr_t end_addr = section->sh_addr + section->sh_size - 1;

        assert(addr % PAGE_SIZE == 0);
        assert(addr <= end_addr);

        uintptr_t flags = present_bit;

        if(!elf_section_is_exectuable(section)) {
            flags |= no_exec_bit;
        }

        if(elf_section_is_writable(section)) {
            flags |= writeable_bit;
        }

        terminal_printf("Addr: %#zx\tFlags : %#zX\n", addr, flags);

        while(addr < end_addr) {
            struct frame frame;
            get_frame_for_addr(&frame, addr);
            addr += PAGE_SIZE;
            identity_map_page(&frame, flags);
        }
    }

    //id map the vga buffer
    struct frame vga_frame;
    get_frame_for_addr(&vga_frame, 0xb8000);
    identity_map_page(&vga_frame, writeable_bit | present_bit | no_exec_bit);

    //id map the multiboot structure
    for(intptr_t mboot_addr = (intptr_t)data.start; mboot_addr <= data.start + data.start->total_size; mboot_addr += PAGE_SIZE) {
        struct frame mboot_frame;
        get_frame_for_addr(&mboot_frame, mboot_addr);
        identity_map_page(&mboot_frame, present_bit | no_exec_bit);
    }

    terminal_printf("New kernel page tables set up.\n");

    //restore old p4
    set_page_table_entry(&old_p4_table->entries[511], &old_p4_frame, present_bit | writeable_bit);
    flush_tlb();
    terminal_printf("Old p4 entry[511] %#zX\n", p4_table->entries[511]);

    unmap_page(&old_p4_page);

    write_tlb(new_p4_frame.number*PAGE_SIZE);
    flush_tlb();
    
    //create guard page from old p4 frame
    struct page guard_page;
    guard_page.number = old_p4_frame.number;
    unmap_page(&guard_page);

    terminal_printf("End of remap.\n");
}


/* From http://git.qemu.org/?p=qemu.git;a=blob;f=target/i386/monitor.c
  47     monitor_printf(mon, TARGET_FMT_plx ": " TARGET_FMT_plx
  48                    " %c%c%c%c%c%c%c%c%c\n",
  49                    addr,
  50                    pte & mask,
  51                    pte & PG_NX_MASK ? 'X' : '-',
  52                    pte & PG_GLOBAL_MASK ? 'G' : '-',
  53                    pte & PG_PSE_MASK ? 'P' : '-',
  54                    pte & PG_DIRTY_MASK ? 'D' : '-',
  55                    pte & PG_ACCESSED_MASK ? 'A' : '-',
  56                    pte & PG_PCD_MASK ? 'C' : '-',
  57                    pte & PG_PWT_MASK ? 'T' : '-',
  58                    pte & PG_USER_MASK ? 'U' : '-',
  59                    pte & PG_RW_MASK ? 'W' : '-');
  */