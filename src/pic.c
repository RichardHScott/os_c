#include "pic.h"

// Following addresses contains information on how to program the 8259 PIC
// http://stanislavs.org/helppc/8259.html
// http://wiki.osdev.org/PIC#Programming_the_PIC_chips

enum pic_addresses {
    master_pic_command = 0x0020,
    master_pic_data = 0x0021,
    slave_pic_command= 0x00A0,
    slave_pic_data = 0x00A1
};

#define PIC_EOI 0x20

enum pic_icw1 {
    pic_icw4_needed = 0x1,
    pic_single_ICW1 = 0x2,
    pic_four_byte_interrupt_vectors = 0x4,
    pic_level_triggered_mode =  0x8,
    pic_ICW1_use = 0x10
};

enum pic_icw4 {
    pic_x86_mode = 0x1
};

// Write to a port that is unused during PIC initialisation
static inline void io_wait() {
    outb(0x80, 0);
}

static inline void pic_start_interrupts() {
    asm volatile ("sti" ::);
}

void irq_set_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;
 
    if(irq_line < 8) {
        port = master_pic_data;
    } else {
        port = slave_pic_data;
        irq_line -= 8;
    }
    value = inb(port) | (1 << irq_line);
    outb(port, value);        
}

void irq_clear_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;
 
    if(irq_line < 8) {
        port = master_pic_data;
    } else {
        port = slave_pic_data;
        irq_line -= 8;
    }
    value = inb(port) & ~(1 << irq_line);
    outb(port, value);        
}

static uint8_t master_remap;
static uint8_t slave_remap;

void pic_init(uint8_t master_remap_offset, uint8_t slave_remap_offset) {
    uint8_t master_mask = inb(master_pic_data);
    uint8_t slave_mask = inb(slave_pic_data);

    outb(master_pic_command, pic_ICW1_use + pic_icw4_needed);
    io_wait();
    outb(slave_pic_command, pic_ICW1_use + pic_icw4_needed);
    io_wait();

    outb(master_pic_data, master_remap_offset);
    io_wait();
    outb(slave_pic_data, slave_remap_offset);
    io_wait();

    outb(master_pic_data, 4);
    io_wait();
    outb(slave_pic_data, 2);
    io_wait();

    outb(master_pic_data, pic_x86_mode);
    io_wait();
    outb(slave_pic_data, pic_x86_mode);
    io_wait();

    outb(master_pic_data, master_mask);
    outb(slave_pic_data, slave_mask);


}

void add_interrupt_handler(uint16_t interrupt, intptr_t handler) {
    asm_add_interrupt_handler(interrupt, handler);
}

void pic_write_EOI(uint8_t irq) {
    if(irq > 8) {
        outb(slave_pic_command, PIC_EOI);
    }
    outb(master_pic_command, PIC_EOI);
}

void pic_enable_interrupts(void) {
    pic_start_interrupts();

    outb(0x64, 0x60);
    outb(0x60, 0x1);

    irq_clear_mask(1);
}

void pic_disable_interrupts(void) {

}