#pragma once
#include <stdint.h>

// void add_interrupt_handler(uint16_t interrupt, void (*handler)(void));

// add_interrupt_handler(uint16_t interrupt, void (*handler)(void)) {
//     asm_add_interrupt_handler(interrupt, handler);
// }

void pic_init(uint8_t master_remap_offset, uint8_t slave_remap_offset);