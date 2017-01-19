#include "multiboot.h"
#include "terminal.h"

void init_multiboot_data(uintptr_t pmultiboot) {
    start_ptr = pmultiboot;
    start = pmultiboot;
    print_text("Total size: ");
    print_hex_number(start->total_size);
    print_newline();

    print_text("Reserved");
    print_hex_number(start->reserved);
    print_newline();

    parse_multiboot_data(start);
}

void parse_multiboot_data(uintptr_t pstart) {
    struct multiboot_tag* curr_tag = pstart + sizeof(struct multiboot_start);
    do {
        //we have a next tag
        switch(curr_tag->type) {
            case multiboot_end_tag:
                print_text("End tag\n");
                break;
            case multiboot_memory_info_tag:
                print_text("Mem info tag\n");
                {
                struct multiboot_memory_information* info = (struct multiboot_memory_information*) curr_tag;
                print_text("mem_lower: ");
                print_hex_number(info->mem_lower);
                print_text("  mem_upper: ");
                print_hex_number(info->mem_upper);
                print_newline();
                }
                break;
            case multiboot_boot_load_name_tag:
                {
                print_text("boot load name\n");
                struct multiboot_boot_load_name* name = (struct multiboot_boot_load_name*) curr_tag;
                print_text(name->string);
                print_newline();
                }
                break;
            case multiboot_bios_boot_device_tag:
                {
                print_text("Bios boot device\n");
                struct multiboot_bios_boot_device* device = (struct multiboot_bios_boot_device*) curr_tag;
                //TODO
                }
                break;
            case multiboot_boot_command_line_tag:
                {
                print_text("Boot command line\n");
                struct multiboot_boot_command_line* command_line = (struct multiboot_boot_command_line*) curr_tag;
                print_text(command_line->string);
                print_newline();
                }
                break;
            case multiboot_modules_tag:
                print_text("Modules tag\n");
                break;
            case multiboot_elf_symbols_tag:
                {
                print_text("Elf symbols\n");
                struct multiboot_elf_symbols* symbols = (struct multiboot_elf_symbols*) curr_tag;
                
                struct multiboot_elf_section_header *string_table = (struct multiboot_elf_section_header*) ((uintptr_t)symbols + 5*sizeof(uint32_t) + symbols->shndx*symbols->entsize);

                for(int i=0; i<symbols->num; ++i) {
                    struct multiboot_elf_section_header* elf_header = (struct multiboot_elf_section_header*) ((uintptr_t)symbols + 5*sizeof(uint32_t) + i*symbols->entsize);

                    print_text((char*)(string_table->sh_addr) + elf_header->sh_name);
                    print_newline();
                    print_hex_number(elf_header->sh_type);
                    print_newline();
                    print_hex_number(elf_header->sh_name);
                    print_newline();
                    for(int k=0;k<1000000000;++k){}
                }

                print_hex_number(symbols->shndx);
                }
                break;
            case multiboot_memory_map_tag:
                {
                print_text("Memory map tag\n");
                struct multiboot_memory_map* maps = (struct multiboot_memory_map*) curr_tag;
                
                int num_of_entries = ((uintptr_t)(maps->size) - 4*sizeof(uint32_t))/maps->entry_size;
                for(int i=0; i<num_of_entries;++i) {
                    struct multiboot_memory_map_entry* entry = (struct multiboot_memory_map_entry*) ((uintptr_t)(maps) + 4*sizeof(uint32_t) + i*sizeof(struct multiboot_memory_map_entry));
                    print_text("Base addr: ");
                    print_hex_uint64(entry->base_addr);
                    print_newline();
                    print_text("Length: ");
                    print_hex_uint64(entry->length);
                    print_newline();
                    print_text("Type: ");
                    print_hex_number(entry->type);
                    print_newline();
                }
                }
                break;
            case multiboot_apm_table_tag:
                print_text("APM table tag\n");
                break;
            case multiboot_vbe_info_tag:
                print_text("VBE info tag\n");
                break;
            case multiboot_framebuffer_info_tag:
                print_text("Framebuffer tag\n");
                break;
            case multiboot_EFI32_system_table_tag:
                print_text("EFI32_system_table_tag tag\n");
                break;
            case multiboot_EFI64_system_table_tag:
                print_text("EFI64_system_table_tag tag\n");
                break;
            case multiboot_SMSBIOS_tables_tag:
                print_text("SMSBIOS_tables_tag tag\n");
                break;
            case multiboot_ACPI_old_RSDP_tag:
                print_text("ACPI_old_RSDP_tag tag\n");
                break;
            case multiboot_ACPI_new_RSDP_tag:
                print_text("ACPI_new_RSDP_tag tag\n");
                break;
            case multiboot_networking_information_tag:
                print_text("networking_information_tag tag\n");
                break;
            case multiboot_EFI_memory_map_tag:
                print_text("EFI_memory_map_tag tag\n");
                break;
            case multiboot_EFI_boot_services_tag:
                print_text("EFI_boot_services_tag tag\n");
                break;
            case multiboot_EFI32_image_ptr_tag:
                print_text("EFI32_image_ptr_tag tag\n");
                break;
            case multiboot_EFI64_image_ptr_tag:
                print_text("EFI64_image_ptr_tag tag\n");
                break;
            case multiboot_image_load_base_phys_addr_tag:
                print_text("image_load_base_phys_add tag\n");
                break;

            default:
                print_text("\nFailed with tag: ");
                print_hex_number(curr_tag);
                print_text("\nTag type:");
                print_hex_number(curr_tag->type);
                print_text("\nTag size:");
                print_hex_number(curr_tag->size);
                print_newline();
                break;
        }
    } while((curr_tag = find_next_tag_address(curr_tag)) != NULL);  
}

void print_tag(struct multiboot_tag* curr_tag) {
    print_text("\nTag address: ");
    print_hex_number(curr_tag);
    print_text("\nTag type:");
    print_hex_number(curr_tag->type);
    print_text("\nTag size:");
    print_hex_number(curr_tag->size);
    print_newline();
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
