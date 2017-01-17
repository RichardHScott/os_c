#include <stdint.h>

// https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

typedef int bit;

typedef struct {
    bit mem:1;
    bit boot_device:1;
    bit cmd_line:1;
    bit mods:1;
    bit symbol_table:1;
    bit elf_header:1; 
    int unused:26;
} Flags __attribute__((packed));

typedef struct {

} SymbolTable __attribute__((packed));

typedef struct {

} ElfHeader  __attribute__((packed));

typedef struct {

} Syms __attribute__((packed));

typedef struct {
    uint8_t part0;
    uint8_t part1;
    uint8_t part2;
    uint8_t part3;
} Bootdevice __attribute__((packed));

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
} Mods __attribute__((packed));

typedef struct {
    Flags flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    Bootdevice boot_device;
    uint32_t cmdline;
    uint32_t mods_count
    uint32_t mods_addr
    Syms syms;
    
} MultibootHeader __attribute__((packed));