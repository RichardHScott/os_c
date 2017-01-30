#pragma once

#include <stdint.h>
#include <stdbool.h>

// multiboot2 header structure
// http://nongnu.askapache.com/grub/phcoder/multiboot.pdf

struct multiboot_start {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_memory_information{
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_bios_boot_device {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t partition;
    uint32_t subpartition;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_boot_command_line {
    uint32_t type;
    uint32_t size;
    uint8_t string[]; //zero terminated utf8 string
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_module {
    uint32_t type;
    uint32_t size;
    uintptr_t mod_start;
    uintptr_t mod_end;
    uint8_t string[];
} __attribute__((packed)) __attribute__ ((aligned (8)));

// Elf header info from
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
enum elf_sh_type {
    sht_null = 0,
    sht_progbits = 1,
    sht_symtab = 2,
    sht_strtab = 3,
    sht_rela = 4,
    sht_hash = 5,
    sht_dynamic = 6,
    sht_note = 7,
    sht_nobits = 8,
    sht_rel = 9,
    sht_shlib = 10,
    sht_dynsym = 11,
    sht_init_array = 12,
    sht_fini_array = 13,
    sht_preinit_array = 14,
    sht_group = 15,
    sht_symtab_shndx = 16,
    sht_num = 17,
    sht_loos = 0x60000000
};

enum ef_sh_flags {
    shf_write = 0x1,
    shf_alloc = 0x2,
    shf_execinstr = 0x4,
    shf_merge = 0x10,
    shf_strings = 0x20,
    shf_info_link = 0x40,
    shf_link_order = 0x80,
    shf_os_nonconforming = 0x100,
    shf_group = 0x200,
    shf_tls = 0x400,
    shf_maskos = 0x0ff00000,
    shf_maskproc = 0xf0000000,
    shf_ordered = 0x4000000,
    shf_exclude = 0x8000000
};

struct multiboot_elf_section_header {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags; //64 bit elf header
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} __attribute__ ((packed));

bool elf_section_is_allocated(struct multiboot_elf_section_header *header);

struct multiboot_elf_symbols {
    uint32_t type;
    uint32_t size;
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    struct multiboot_elf_section_header sectionheaders[];
} __attribute__((packed)) __attribute__ ((aligned (8)));

enum multiboot_memory_map_entry_type {
    multiboot_ram_available = 1,
    multiboot_ram_reserved = 2,
    multiboot_acpi = 3,
    multiboot_preserved_on_hiber = 4,
    multiboot_defective_ram = 5
};

struct multiboot_memory_map_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot_memory_map {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_memory_map_entry memory_maps[];
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_boot_loader_name {
    uint32_t type;
    uint32_t size;
    uint8_t string[];
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_apm_table {
    uint32_t type;
    uint32_t size;
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg_16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseg_len;
    uint16_t cseg_16_len;
    uint16_t dseg_len;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_vbe_info {
    uint32_t type;
    uint32_t size;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint8_t vbe_control_info[512];
    uint8_t vbe_mode_info[256];
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_framebuffer_palette {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} __attribute__((packed));

struct multiboot_color_type_indexed {
    uint32_t number_of_colors;
    struct multiboot_framebuffer_palette palette[];
} __attribute__((packed));

struct multiboot_color_type_direct_rgb {
    uint8_t red_field_position;
    uint8_t red_mask_size;
    uint8_t green_field_position;
    uint8_t green_mask_size;
    uint8_t blue_field_position;
    uint8_t blue_mask_size;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_framebuffer_info {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved;
    uint8_t* color_info; //either a color_type_indexed or color_type_direct
                         //TODO: 2017/01/17 fix this to correct change the color info;
} __attribute__((packed)) __attribute__ ((aligned (8)));

struct multiboot_boot_load_name {
    uint32_t type;
    uint32_t size;
    uint8_t string[];
} __attribute__((packed)) __attribute__((aligned (8)));

uintptr_t start_ptr;

struct multiboot_start* start;
struct multiboot_memory_information *mem_info;

struct multiboot_data {
    struct multiboot_start *start;
    struct multiboot_memory_map *memory_map;
    struct multiboot_elf_symbols *elf_symbols;
};

struct multiboot_data data;

void init_multiboot_data(uintptr_t pmultiboot);
void parse_multiboot_data(uintptr_t pstart);
uintptr_t find_next_tag_address(struct multiboot_tag* curr_tag);

enum multiboot_tags {
    multiboot_end_tag = 0,
    multiboot_boot_command_line_tag = 1,
    multiboot_boot_load_name_tag = 2,
    multiboot_modules_tag = 3,
    multiboot_memory_info_tag = 4,
    multiboot_bios_boot_device_tag = 5,
    multiboot_memory_map_tag = 6,
    multiboot_elf_symbols_tag = 9,
    multiboot_apm_table_tag = 10,
    multiboot_vbe_info_tag = 7,
    multiboot_framebuffer_info_tag = 8,
    multiboot_EFI32_system_table_tag = 11,
    multiboot_EFI64_system_table_tag = 12,
    multiboot_SMSBIOS_tables_tag = 13,
    multiboot_ACPI_old_RSDP_tag = 14, //???
    multiboot_ACPI_new_RSDP_tag = 15, //???
    multiboot_networking_information_tag = 16,
    multiboot_EFI_memory_map_tag = 17,
    multiboot_EFI_boot_services_tag = 18,
    multiboot_EFI32_image_ptr_tag = 19,
    multiboot_EFI64_image_ptr_tag = 20,
    multiboot_image_load_base_phys_addr_tag = 21
};
