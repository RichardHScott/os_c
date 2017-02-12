#include "interrupts.h"

#define IDT_ENTRY_COUNT 256

struct IDT {
    struct interrupt_description_table_entry entries[IDT_ENTRY_COUNT];
};

struct IDT *idt; 

static uint64_t get_handler_address(struct interrupt_description_table_entry* table) {
    return table->low_offset | (table->mid_offset << 16) | (table->high_offset << 32);
}

static void set_handler_address(struct interrupt_description_table_entry* table, uint64_t addr) {
    uint16_t low_addr = addr & 0xffff;
    uint16_t mid_addr = (addr >> 16) & 0xffff;
    uint32_t high_addr = (addr >> 32);

    table->low_offset = low_addr;
    table->mid_offset = mid_addr;
    table->high_offset = high_addr;
}

static void interrupt_print(void) {
    for(int i=0; i < IDT_ENTRY_COUNT; ++i) {
        struct interrupt_description_table_entry *table = &idt->entries[i];
        
        terminal_printf("Flags: \n");
        terminal_printf("Segment selector: %d\n", table->segement_selector);
        terminal_printf("Handler address: %#zX\n", get_handler_address(table));
    }
}

void set_idt(intptr_t addr) {
    idt = (struct IDT*) addr;
}

static void set_handler(struct IDT* table, intptr_t addr, enum idt_gate_type type) {

}

typedef struct intr_frame {
  uint64_t ip;
  uint64_t cs;
  uint64_t flags;
  uint64_t sp;
  uint64_t ss;
} intr_frame;

__attribute__ ((interrupt))
void handle_something(struct intr_frame *frame)  {
    int i =0;
    ++i;
}