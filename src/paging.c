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

/*
Bit	Name	Full Name	Description
0	PE	Protected Mode Enable	If 1, system is in protected mode, else system is in real mode
1	MP	Monitor co-processor	Controls interaction of WAIT/FWAIT instructions with TS flag in CR0
2	EM	Emulation	If set, no x87 floating point unit present, if clear, x87 FPU present
3	TS	Task switched	Allows saving x87 task context upon a task switch only after x87 instruction used
4	ET	Extension type	On the 386, it allowed to specify whether the external math coprocessor was an 80287 or 80387
5	NE	Numeric error	Enable internal x87 floating point error reporting when set, else enables PC style x87 error detection
16	WP	Write protect	When set, the CPU can't write to read-only pages when privilege level is 0
18	AM	Alignment mask	Alignment check enabled if AM set, AC flag (in EFLAGS register) set, and privilege level is 3
29	NW	Not-write through	Globally enables/disable write-through caching
30	CD	Cache disable	Globally enables/disable the memory cache
31	PG	Paging	If 1, enable paging and use the CR3 register, else disable paging
*/

enum cr0_bits {
    cr0_protected_mode_enable = 1,
    cr0_monitor_co_processor = 1 << 1,
    cr0_emulation = 1 << 2,
    cr0_task_switched = 1 << 3,
    cr0_extension_type = 1 << 4,
    cr0_numeric_error = 1 << 5,
    cr0_write_protect = 1 << 16,
    cr0_alignment_mask = 1 << 18,
    cr0_not_write_through = 1 << 29,
    cr0_cache_disable = 1 << 30,
    cr0_paging = 1 << 31
};

/*
Bit	Name	Full Name	Description
0	VME	Virtual 8086 Mode Extensions	If set, enables support for the virtual interrupt flag (VIF) in virtual-8086 mode.
1	PVI	Protected-mode Virtual Interrupts	If set, enables support for the virtual interrupt flag (VIF) in protected mode.
2	TSD	Time Stamp Disable	If set, RDTSC instruction can only be executed when in ring 0, otherwise RDTSC can be used at any privilege level.
3	DE	Debugging Extensions	If set, enables debug register based breaks on I/O space access
4	PSE	Page Size Extension	If unset, page size is 4 KiB, else page size is increased to 4 MiB (if PAE is enabled or the processor is in Long Mode this bit is ignored[1]).
5	PAE	Physical Address Extension	If set, changes page table layout to translate 32-bit virtual addresses into extended 36-bit physical addresses.
6	MCE	Machine Check Exception	If set, enables machine check interrupts to occur.
7	PGE	Page Global Enabled	If set, address translations (PDE or PTE records) may be shared between address spaces.
8	PCE	Performance-Monitoring Counter enable	If set, RDPMC can be executed at any privilege level, else RDPMC can only be used in ring 0.
9	OSFXSR	Operating system support for FXSAVE and FXRSTOR instructions	If set, enables SSE instructions and fast FPU save & restore
10	OSXMMEXCPT	Operating System Support for Unmasked SIMD Floating-Point Exceptions	If set, enables unmasked SSE exceptions.
13	VMXE	Virtual Machine Extensions Enable	see Intel VT-x
14	SMXE	Safer Mode Extensions Enable	see Trusted Execution Technology (TXT)
16	FSGSBASE	Enables the instructions RDFSBASE, RDGSBASE, WRFSBASE, and WRGSBASE.
17	PCIDE	PCID Enable	If set, enables process-context identifiers (PCIDs).
18	OSXSAVE	XSAVE and Processor Extended States Enable	
20	SMEP[2]	Supervisor Mode Execution Protection Enable	If set, execution of code in a higher ring generates a fault
21	SMAP	Supervisor Mode Access Protection Enable	If set, access of data in a higher ring generates a fault[3]
22	PKE	Protection Key Enable	See Intel® 64 and IA-32 Architectures Software Developer’s Manual
*/

enum cr4_bits {
    cr4_vme = 1,
    cr4_pvi = 1 << 1,
    cr4_tsd = 1 << 2,
    cr4_de = 1 << 3,
    cr4_pse = 1 << 4,
    cr4_pae = 1 << 5,
    cr4_mce = 1 << 6,
    cr4_pge = 1 << 7,
    cr4_pce = 1 << 8,
    cr4_osfxsr = 1 << 9,
    cr4_osxmmecpt = 1 << 10,
    cr4_vmxe = 1 << 13,
    cr4_smxe = 1 << 14,
    cr4_fsgsbase = 1 << 15,
    cr4_pcide = 1 << 17,
    cr4_osxsave = 1 << 18,
    cr4_smep = 1 << 20,
    cr4_smap = 1 << 21,
    cr4_pke = 1 << 22
};

/*
Bit	Purpose
0	SCE (System Call Extensions)
1–7	Reserved
8	LME (Long Mode Enable)
9	Reserved
10	LMA (Long Mode Active)
11	NXE (No-Execute Enable)
12	SVME (Secure Virtual Machine Enable)
13	LMSLE (Long Mode Segment Limit Enable)
14	FFXSR (Fast FXSAVE/FXRSTOR)
15	TCE (Translation Cache Extension)
16–63	Reserved
*/

const uint32_t ia32_msfr = 0xC0000080;

enum efer_bits {
    efer_sce = 1,
    efer_lme = 1 << 8,
    efer_lma = 1 << 10,
    efer_nxe = 1 << 11,
    efer_svme = 1 << 12,
    efer_lmsle = 1 << 13,
    efer_ffxsr = 1 << 14,
    efer_tce = 1 << 15
};

static uint64_t read_efer(uint32_t msr) {
    uint32_t low, high;
    asm volatile("mov %2, %%ecx\n"
                 "rdmsr\n"
                 "mov %%eax, %0\n"
                 "mov %%edx, %1\n"
                : "=r"(low), "=r"(high)
                : "r"(msr)
                : "%ecx", "%rax", "%rdx");
    return low | (high << 32);
}

static void write_efer(uint64_t val, uint32_t msr) {
    uint32_t low = val & 0xffffffff;
    uint32_t high = val >> 32;

    asm volatile("mov %0, %%ecx\n"
                 "mov %1, %%eax\n"
                 "mov %2, %%edx\n"
                 "wrmsr\n"
                :
                : "r"(msr), "r"(low), "r"(high)
                : "%ecx", "%rax", "%rdx");
}

static int64_t read_cr0() {
    int64_t val;
    asm volatile ("mov %%cr0, %0" : "=r"(val));
    return val;
}

static void write_cr0(int64_t val) {
    asm volatile ("mov %0, %%cr0" : : "r"(val));
}

static int64_t read_cr4() {
    int64_t val;
    asm volatile ("mov %%cr4, %0" : "=r"(val));
    return val;
}

static void write_cr4(int64_t val) {
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
    //enable_write_protect(); //currently breaks as it makes stack unwritable

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