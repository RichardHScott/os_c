#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "assert.h"

static const size_t COL_MAX = 25;
static const size_t ROW_MAX = 80;

typedef enum {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
} vga_color_code;

typedef uint8_t vga_color;
typedef uint16_t vga_char;

typedef struct {
	size_t col;
	size_t row;

	vga_color default_color;
	vga_color color;

	uint16_t* buffer;

	size_t col_max;
	size_t row_max;
} Terminal;

void clear_terminal(void);
void init_terminal(void);
void print_text(const char* text);
void set_foreground_color(vga_color_code new_color);
void set_background_color(vga_color_code new_color);
void terminal_print_char(char c);

void terminal_printf(const char* str, ...);