#include "multiboot.h"
#include "terminal.h"

struct multiboot_memory_information* info;
struct multiboot_boot_load_name* name;
struct multiboot_bios_boot_device* device;
struct multiboot_boot_command_line* command_line;
struct multiboot_elf_symbols* symbols;
struct multiboot_memory_map* maps;

void init_multiboot_data(uintptr_t pmultiboot) {
    start_ptr = pmultiboot;
    start = pmultiboot;

    terminal_printf("Multiboot addr: %#zx \t Total size: %#zx\n", start_ptr, start->total_size);

    data.start = pmultiboot;

    parse_multiboot_data(start);
}

void parse_multiboot_data(uintptr_t pstart) {
    struct multiboot_tag* curr_tag = pstart + sizeof(struct multiboot_start);
    do {
        //we have a next tag
        switch(curr_tag->type) {
            case multiboot_end_tag:
                terminal_printf("End tag\n");
                break;
            case multiboot_memory_info_tag:
                terminal_printf("Mem info tag\n");
                {
                info = (struct multiboot_memory_information*) curr_tag;
                terminal_printf("mem_lower: %#zx \t mem_upper: %#zx\n", info->mem_lower, info->mem_upper);
                }
                break;
            case multiboot_boot_load_name_tag:
                {
                name = (struct multiboot_boot_load_name*) curr_tag;
                terminal_printf("boot load name: %s\n", name->string);
                }
                break;
            case multiboot_bios_boot_device_tag:
                {
                terminal_printf("Bios boot device\n");
                device = (struct multiboot_bios_boot_device*) curr_tag;
                //TODO
                }
                break;
            case multiboot_boot_command_line_tag:
                {
                command_line = (struct multiboot_boot_command_line*) curr_tag;
                terminal_printf("Boot command line: %s\n", command_line->string);
                }
                break;
            case multiboot_modules_tag:
                terminal_printf("Modules tag\n");
                break;
            case multiboot_elf_symbols_tag:
                {
                terminal_printf("Elf symbols\n");
                symbols = (struct multiboot_elf_symbols*) curr_tag;
                data.elf_symbols = symbols;
                }
                break;
            case multiboot_memory_map_tag:
                {
                terminal_printf("Memory map tag\n");
                maps = (struct multiboot_memory_map*) curr_tag;
                data.memory_map = maps;

                int num_of_maps = (maps->size - 4*sizeof(uint32_t))/maps->entry_size;

                for(int i=0; i<num_of_maps; ++i) {
                    struct multiboot_memory_map_entry mem_map = maps->memory_maps[i];
                    terminal_printf("Addr: %#zx \t Length: %#zx\n", mem_map.base_addr, mem_map.length);
                }
                }
                break;
            case multiboot_apm_table_tag:
                terminal_printf("APM table tag\n");
                break;
            case multiboot_vbe_info_tag:
                terminal_printf("VBE info tag\n");
                break;
            case multiboot_framebuffer_info_tag:
                terminal_printf("Framebuffer tag\n");
                break;
            case multiboot_EFI32_system_table_tag:
                terminal_printf("EFI32_system_table_tag tag\n");
                break;
            case multiboot_EFI64_system_table_tag:
                terminal_printf("EFI64_system_table_tag tag\n");
                break;
            case multiboot_SMSBIOS_tables_tag:
                terminal_printf("SMSBIOS_tables_tag tag\n");
                break;
            case multiboot_ACPI_old_RSDP_tag:
                terminal_printf("ACPI_old_RSDP_tag tag\n");
                break;
            case multiboot_ACPI_new_RSDP_tag:
                terminal_printf("ACPI_new_RSDP_tag tag\n");
                break;
            case multiboot_networking_information_tag:
                terminal_printf("networking_information_tag tag\n");
                break;
            case multiboot_EFI_memory_map_tag:
                terminal_printf("EFI_memory_map_tag tag\n");
                break;
            case multiboot_EFI_boot_services_tag:
                terminal_printf("EFI_boot_services_tag tag\n");
                break;
            case multiboot_EFI32_image_ptr_tag:
                terminal_printf("EFI32_image_ptr_tag tag\n");
                break;
            case multiboot_EFI64_image_ptr_tag:
                terminal_printf("EFI64_image_ptr_tag tag\n");
                break;
            case multiboot_image_load_base_phys_addr_tag:
                terminal_printf("image_load_base_phys_add tag\n");
                break;

            default:
                terminal_printf("Failed with tag: %#zx \t Type: %#zx \t Size: %#zx\n", curr_tag, curr_tag->type, curr_tag->size);
                break;
        }
    } while((curr_tag = find_next_tag_address(curr_tag)) != NULL);

    //print_elf_symbols();
}

const char *ef_sh_flags_str[] = { "", "W", "A", "X", "M", "S", "IL", "LO", "OS", "G", "T", "MO", "MP", "ORD", "EXC" };

char* get_sh_flag_string(enum ef_sh_flags flag) {
    int index = 0;

    switch(flag) {
        case shf_write:
            index = 1;
            break;
        case shf_alloc:
            index = 0x2;
            break;
        case shf_execinstr:
            index = 0x3;
            break;
        case shf_merge:
            index = 0x4;
            break;
        case shf_strings:
            index = 0x5;
            break;
        case shf_info_link:
            index = 0x6;
            break;
        case shf_link_order:
            index = 0x7;
            break;
        case shf_os_nonconforming:
            index = 0x8;
            break;
        case shf_group:
            index = 0x9;
            break;
        case shf_tls:
            index = 0xa;
            break;
        case shf_maskos:
            index = 0xb;
            break;
        case shf_maskproc:
            index = 0xc;
            break;
        case shf_ordered:
            index = 0xd;
            break;
        case shf_exclude:
            index = 0xe;
            break;
    }

    return ef_sh_flags_str[index];
}

void print_memory_tags(void) {
    int num_of_entries = ((uintptr_t)(maps->size) - 4*sizeof(uint32_t))/maps->entry_size;

    for(int i=0; i<num_of_entries;++i) {
        struct multiboot_memory_map_entry* entry = (struct multiboot_memory_map_entry*) ((uintptr_t)(maps) + 4*sizeof(uint32_t) + i*sizeof(struct multiboot_memory_map_entry));
        terminal_printf("Addr: %#zx \t Length: %#zx \t Type: %#zx\n", entry->base_addr, entry->length, entry->type);
    }
}

void print_elf_symbols(void) {
    struct multiboot_elf_section_header *string_table = (struct multiboot_elf_section_header*) ((uintptr_t)symbols + 5*sizeof(uint32_t) + symbols->shndx*symbols->entsize);

    for(int i=0; i<symbols->num; ++i) {
        struct multiboot_elf_section_header* elf_header = (struct multiboot_elf_section_header*) ((uintptr_t)symbols + 5*sizeof(uint32_t) + i*symbols->entsize);
        terminal_printf("Addr: %#zx \t Length: %#zx \t Flags: ", elf_header->sh_addr, elf_header->sh_type);

        for(size_t i=1; i<(SIZE_MAX>>1); i=i<<1) {
            enum ef_sh_flags flag = i & elf_header->sh_flags;
            if(flag != 0) {
                terminal_printf("%s ", get_sh_flag_string(flag));
            }
        }
        terminal_printf("Size: %#zx %s\n", elf_header->sh_size, (char*)(string_table->sh_addr) + elf_header->sh_name);
    }
}

void print_tag(struct multiboot_tag* curr_tag) {
    terminal_printf("Tag address: %#zx \t Type: %#zx \t Size: %#zx\n", curr_tag, curr_tag->type, curr_tag->size);
}

uintptr_t find_next_tag_address(struct multiboot_tag* curr_tag) {
    if(curr_tag->type != 0) {
        uintptr_t next_tag = (uintptr_t)curr_tag + curr_tag->size;
        if(next_tag % 8 != 0) {
            next_tag += 8 - next_tag % 8; 
        }
        return next_tag;
    } else {
        return NULL;
    }
}
