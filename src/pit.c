#include "pit.h"

const uint8_t pit_channel0_data_port = 0x40;
const uint8_t pit_channel1_data_port = 0x41;
const uint8_t pit_channel2_data_port = 0x42;
const uint8_t pit_command_register_port = 0x43;

typedef uint8_t pit_cmd_word_t;

static uint8_t pit_read_cmd_word(void) {
    return inb(pit_command_register_port);
}

static void pit_write_cmd_word(uint8_t port, uint16_t word) {
    outb(port, word & 0xf);
    outb(port, (word & 0xf0) >> 8);
}

static void pit_write_cmd_byte(uint8_t port, uint8_t byte) {
    outb(port, byte);
}

void pit_set_pit0_freq(uint16_t new_divider) {
    pit_write_cmd_word(pit_channel0_data_port, new_divider);
}