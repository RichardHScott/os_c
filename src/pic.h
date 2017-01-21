#pragma once
#include <stdint.h>
#include "util.h"

void add_interrupt_handler(uint16_t interrupt, uintptr_t handler);

void pic_init(uint8_t master_remap_offset, uint8_t slave_remap_offset);

void pic_enable_interrupts(void);
void pic_disable_interrupts(void);