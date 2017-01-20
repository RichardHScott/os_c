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

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
    return ret;
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0"
                  : "=a"(ret)
                  : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1"
                  : 
                  : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1"
                  : 
                  : "a"(val), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outb %0, %1"
                  : 
                  : "a"(val), "Nd"(port));
}

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

    pic_start_interrupts();

    outb(0x64, 0x60);
    outb(0x60, 0x1);

    irq_clear_mask(1);
}

pic_write_EOI(uint8_t irq) {
    if(irq > 8) {
        outb(slave_pic_command, PIC_EOI);
    }
    outb(master_pic_command, PIC_EOI);
}
