#pragma once

#include <stdint.h>

// 6.14.1 64-Bit Mode IDT, p253 
// http://www.intel.com/Assets/en_US/PDF/manual/253668.pdf
struct interrupt_description_table_entry {
    uint16_t low_offset;
    uint32_t segement_selector;
    uint32_t flags;
    uint16_t mid_offset;
    uint32_t high_offset;
    uint32_t reserved;
} __attribute__ ((packed));

enum idt_flags {
    idt_ist_mask = 0x7,
    idt_type = 1 << 9 + 1 << 10 + 1 << 11,
    idt_dpl = 1 << 13 + 1 << 14,
    idt_present = 1 << 15
};

enum idt_gate_type {
    idt_gate_reserved = 0
};

/*
Type Field Description
Decimal 11 10 9 8 32-Bit Mode IA-32e Mode
0 0 0 0 0 Reserved Upper 8 byte of an 16-byte descriptor
1 0 0 0 1 16-bit TSS (Available) Reserved
2 0 0 1 0 LDT LDT
3 0 0 1 1 16-bit TSS (Busy) Reserved
4 0 1 0 0 16-bit Call Gate Reserved
5 0 1 0 1 Task Gate Reserved
6 0 1 1 0 16-bit Interrupt Gate Reserved
7 0 1 1 1 16-bit Trap Gate Reserved
8 1 0 0 0 Reserved Reserved
9 1 0 0 1 32-bit TSS (Available) 64-bit TSS (Available)
10 1 0 1 0 Reserved Reserved
11 1 0 1 1 32-bit TSS (Busy) 64-bit TSS (Busy)
12 1 1 0 0 32-bit Call Gate 64-bit Call Gate
13 1 1 0 1 Reserved Reserved
14 1 1 1 0 32-bit Interrupt Gate 64-bit Interrupt Gate
15 1 1 1 1 32-bit Trap Gate 64-bit Trap Gate
*/