#include "terminal.h"

static Terminal terminal;

static inline vga_color make_vga_color(vga_color_code fg, vga_color_code bg) {
	return fg | bg << 4;
}
 
static inline vga_char make_vga_char(unsigned char uc, vga_color color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

static inline void terminal_put_char_at(vga_char c, size_t x, size_t y) {
	terminal.buffer[x + y * terminal.row_max] = c;
}

void init_terminal(void) {
	terminal.col = 0;
	terminal.row = 0;
	terminal.buffer = (uint16_t*) 0xB8000;
	terminal.default_color = make_vga_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
	terminal.color = terminal.default_color;
	terminal.col_max = COL_MAX;
	terminal.row_max = ROW_MAX;

	clear_terminal();

	print_text("Terminal init completed.\n");
}

void clear_terminal(void) {
	for(size_t x=0; x < terminal.row_max; ++x) {
		for(size_t y=0; y < terminal.col_max; ++y) {
			terminal_put_char_at(make_vga_char(' ', terminal.default_color), x, y);
		}
	}
}

size_t strlen(const char* s) {
	size_t len=0;
	while(s[len]) {
		++len;
	}

	return len;
}

void scroll_terminal(void) {
	for(size_t x=0; x < terminal.row_max; x++) {
		for(size_t y=0; y < terminal.col_max-1; y++) {
			terminal.buffer[x + y * terminal.row_max] = terminal.buffer[x + (1+y) * terminal.row_max];
		}
	}

	for(size_t x=0; x < terminal.row_max; x++) {
		terminal_put_char_at(make_vga_char(' ', terminal.default_color), x, terminal.col_max-1);
	}
}

void print_newline(void) {
	terminal.row = 0;
	terminal.col++;
	if(terminal.col == terminal.col_max) {
		scroll_terminal();
		terminal.col--;
	}
}

void terminal_put_char(vga_char c) {
	terminal_put_char_at(c, terminal.row, terminal.col);

	++terminal.row;
	if(terminal.row == terminal.row_max) {
		terminal.row = 0;
		print_newline();
	}
}

static const char newline = '\n';

void print_char(char c) {
	if(c == newline) {
		print_newline();
	} else {
		terminal_put_char(make_vga_char(c, terminal.default_color));
	}
}

void print_text(const char* text) {
	for(size_t i=0; i < strlen(text); ++i) {
		if(text[i] == newline) {
			print_newline();
		} else {
			terminal_put_char(make_vga_char(text[i], terminal.default_color));
		}
	}
}

void print_hex_number(uint32_t num) {
	char buf[11] = "0x00000000";

	int i=9;
	while(num > 0) {
		int end_digit = num % 16;
		buf[i--] = (end_digit < 10) ? (end_digit + 0x30) : (end_digit + 0x37);
		num = num >> 4;
	}

	print_text(buf);
}

void print_hex_uint64(uint64_t num) {
	char buf[19] = "0x0000000000000000";

	int i=17;
	while(num > 0) {
		int end_digit = num % 16;
		buf[i--] = (end_digit < 10) ? (end_digit + 0x30) : (end_digit + 0x37);
		num = num >> 4;
	}

	print_text(buf);
}