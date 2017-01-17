#include <stdint.h>

// multiboot2 header structure
// http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
typedef int bit;

typedef struct {
    uint32_t total_size;
    uint32_t reserved;
} Flags __attribute__((packed));

typedef enum {
    memory_information = 4,
    bios_boot_device = 5,
    boot_command_line = 1,
    modules = 3,
    elf_symbol = 9,
    memory_map = 6,
    boot_loader_name = 2,
    apm_table = 10,
    vbe_info = 7,
    framebuffer_info = 8
} multiboot_type;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} memory_information __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t partition;
    uint32_t subpartition;
} bios_boot_device __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uint8_t string[]; //zero terminated utf8 string
} boot_command_line __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uintptr_t mod_start;
    uintptr_t mod_end;
    uint8_t string[];
} module __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uint16_t num;
    uint16_t entsize;
    uint16_t shndx;
    uint16_t reserved;
    uint8_t* sectionheaders; //TODO: 2017/01/17 refer to elf header info on how these are laid out
                             //this is a placeholder
} elf_symbols __attribute__((packed));

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} memory_map_entry __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    memory_map_entry* memory_maps;
} memory_map __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uint8_t string[];
} boot_loader_name __attribute__((packed));

typedef struct {
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
} apm_table __attribute__((packed));

typedef struct {
    uint32_t type;
    uint32_t size;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint8_t vbe_control_info[512];
    uint8_t vbe_mode_info[256];
} vbe_info __attribute__((packed));

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} framebuffer_palette __attribute__((packed));

typedef struct {
    uint32_t number_of_colors;
    framebuffer_palette palette[];
} color_type_indexed __attribute__((packed));

typedef struct {
    uint8_t red_field_position;
    uint8_t red_mask_size;
    uint8_t green_field_position;
    uint8_t green_mask_size;
    uint8_t blue_field_position;
    uint8_t blue_mask_size;
} color_type_direct_rgb __attribute__((packed));

typedef struct {
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
} framebuffer_info __attribute__((packed));

