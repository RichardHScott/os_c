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

static void scroll_terminal(void) {
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

void terminal_print_char(char c) {
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

enum types {
	INT, 
	S_CHAR, 
	CHAR, 
	SHORT_INT, 
	LONG_INT, 
	LONG_LONG_INT, 
	INTMAX_T, 
	SIZE_T, 
	PTRDIFF_T, 
	LONG_DOUBLE, 
	WINT_T, 
	WCHAR_T, 
	VOID, 
	DOUBLE, 
	NO_FORMAT
};

enum printf_flags {
	NO_FLAGS = 0x0,
	LEFT_JUSTIFY = 0x01,
	ALWAYS_SHOW_SIGN = 0x02,
	ALWAYS_SHOW_SPACE = 0x04,
	ALWAYS_SHOW_0x_OR_DECIMAL_POINT = 0x08,
	LEFT_PAD_WITH_ZEROS = 0x10
};

enum width_format {
	NONE = 0, FORMAT_ARG_SPECIFIED = -2
};

struct printf_format {
	enum printf_flags flags;
	int width;
	int precision;

	enum types type;
	int is_unsigned;
	int is_pointer;
	char specifier;
};

/* Returns:
 * -1 for error
 * 0 for nothing found
 * Returns integer offset of the next character to be read
 */
static int get_flags(struct printf_format* format, const char* str) {
	int i = 0;
	format->flags = NO_FLAGS;

	switch(str[i]) {
		case '-':
			++i;
			format->flags |= LEFT_JUSTIFY;
			break;
		case '+':
			++i;
			format->flags |= ALWAYS_SHOW_SIGN;
			break;
		case ' ':
			++i;
			format->flags |= ALWAYS_SHOW_SPACE;
			break;
		case '#':
			++i;
			format->flags |= ALWAYS_SHOW_0x_OR_DECIMAL_POINT;
			break;
		case '0':
			++i;
			format->flags |= LEFT_PAD_WITH_ZEROS;
			break;
		default:
			break;
	}

	return i;
}

static inline int read_decimal(const char* str, int* pos) {
	int i = *pos;
	int d = 0;
	int power = 1;

	while(str[i] >= '0' && str[i] <= '9') {
		d += power * (str[i] - '0');
		power *= 10;
		++i;
	}

	*pos = i;
	return d;
}

static int get_width(struct printf_format* format, const char* str) {
	if(str[0] == '*') {
		format->width = FORMAT_ARG_SPECIFIED;
		return 1;
	}
	
	int i=0;
	int spec = read_decimal(&str[i], &i);
	format->width = spec;

	return i;
}

static int get_precision(struct printf_format* format, const char* str) {
	if(str[0] != '.') {
		format->precision = NONE;
		return 0;
	}

	int i=1;
	if(str[i] == '*') {
		format->precision = FORMAT_ARG_SPECIFIED;
	}
	format->precision = read_decimal(&str[i], &i);

	return i;
}

static int get_length_and_specifier(struct printf_format* format, const char* str) {
	int i = 0;

	switch(str[i]) {
		case 'h':
			++i;
			if(str[i] == 'h') {
				format->type = S_CHAR;
				++i;
			} else {
				format->type = SHORT_INT;
			}
			break;
		case 'l':
			++i;
			if(str[i] == 'l') {
				format->type = LONG_LONG_INT;
				++i;
			} else {
				format->type = LONG_INT;
			}
			break;
		case 'j':
			++i;
			format->type = INTMAX_T;
			break;
		case 'z':
			++i;
			format->type = SIZE_T;
			break;
		case 't':
			++i;
			format->type = PTRDIFF_T;
			break;
		case 'L':
			++i;
			format->type = LONG_DOUBLE;
			break;
		default:
			format->type = INT;
			break;
	}

	format->is_unsigned = 0;
	format->is_pointer = 0;
	switch(str[i]) {
		case 'd':
		case 'i':
			assert(format->type != LONG_DOUBLE);
			break;
		case 'u':
		case 'o':
		case 'x':
		case 'X':
			format->is_unsigned = 1;
			break;
		case 'f':
		case 'F':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
		case 'a':
		case 'A':
			assert(format->type == LONG_DOUBLE || format->type == INT);
			if(format->type == INT) {
				format->type = DOUBLE;
			}
			break;
		case 'c':
			assert(format->type == LONG_INT || format->type == INT);
			if(format->type == LONG_INT) {
				format->type = WINT_T;
			}
			break;
		case 's':
			format->is_pointer = 1;
			assert(format->type == LONG_INT || format->type == INT);
			if(format->type == INT) {
				format->type = CHAR;
			} else {
				format->type = WCHAR_T;
			}
			break;
		case 'p':
			format->is_pointer = 1;
			assert(format->type == INT);
			format->type = VOID;
			break;
		case 'n':
			assert(format->type != LONG_DOUBLE);
			format->is_pointer = 1;
		default:
			assert(0!=0);
	}

	format->specifier = str[i];

	return i;
}

static int get_format(struct printf_format* format, const char* str) {
	int i = 0;
	i += get_flags(format, &str[i]);
	assert(i != -1);

	int ret = get_width(format, &str[i]);
	assert(ret != -1);
	i += ret;

	ret = get_precision(format, &str[i]);
	assert(ret != -1);
	i += ret;

	ret = get_length_and_specifier(format, &str[i]);
	assert(ret != -1);
	i += ret;

	return i;
}

static inline int min(int a, int b) {
	if(a < b) {
		return a;
	} else {
		return b;
	}
}

static void print_integer_as_hex(struct printf_format *format, size_t num) {
	char buf[17] = "0000000000000000";

	char offset_for_hex = 0x57;
	if(format->specifier == 'X') {
		offset_for_hex = 0x37;
	}

	int i=15;
	while(num > 0) {
		int end_digit = num % 16;
		buf[i--] = (end_digit < 10) ? (end_digit + 0x30) : (end_digit + offset_for_hex);
		num = num >> 4;
	}

	for(i=0; i < sizeof(buf)/sizeof(buf[0]) - 1; ++i) {
		if(buf[i] != '0') {
			break;
		}
	}

	i = min(i, 15 - format->width);

	if(format->flags & ALWAYS_SHOW_0x_OR_DECIMAL_POINT) {
		if(format->specifier == 'X') {
			print_text("0X");
		} else {
			print_text("0x");
		}
	}

	print_text(&buf[i]);
}

static void print_integer_as_decimal(struct printf_format *format, size_t num) {

}

static void print_integer(struct printf_format *format, size_t num) {
	if(format->specifier == 'X' || format->specifier == 'x') {
		print_integer_as_hex(format, num);
	} else {
		print_integer_as_decimal(format, num);
	}
}

static void print_int(struct printf_format* format, int data) {
	size_t elem = data;
	print_integer(format, elem);
}

static void print_unsigned_int(struct printf_format* format, size_t data) {
	print_integer(format, data);
}

static void print_signed_int(struct printf_format* format, long long int data) {
	print_integer(format, data);
}

static void print_char(struct printf_format* format, char data) {
	terminal_put_char(make_vga_char(data, terminal.default_color));
}

static void print_long_int(struct printf_format* format, long int data) {
	size_t elem = data;
	print_integer(format, elem);
}

static void print_long_long_int(struct printf_format* format, long long int data) {
	size_t elem = data;
	print_integer(format, elem);
}

static void print_intmax_t(struct printf_format* format, intmax_t data) {
	size_t elem = data;
	print_integer(format, elem);
}

static void print_size_t(struct printf_format* format, size_t data) {
	size_t elem = data;
	print_integer(format, elem);
}

static void print_short_int(struct printf_format* format, short int data) {
	size_t elem = data;
	print_integer(format, data);
}

static void print_s_char(struct printf_format* format, unsigned char data) {
	size_t elem = data;
	print_integer(format, data);
}

static void print_ptrdiff_t(struct printf_format* format, ptrdiff_t data) {
	size_t elem = data;
	print_integer(format, elem);
}

static void print_wchar_t(struct printf_format* format, wchar_t data) {

}

static void print_string(struct printf_format* format, const char* data) {
	int i=0;
	while(data[i] != 0) {
		terminal_put_char(make_vga_char(data[i++], terminal.default_color));
	}
}

static void print_long_double(struct printf_format* format, long double data) {

}

static void print_double(struct printf_format* format, double data) {

}

const int tab_space_num = 4;

void terminal_printf(const char* str, ...) {
	int curr_arg = 0;
	va_list args;

	va_start(args, str);

	for(int i=0; i < strlen(str); ++i) {
		if(str[i] == '\\') {
			switch(str[++i]) {
				case '\\':
					break;
			}
		} else if(str[i] == '%') {
			if(str[++i] != '%') {
				struct printf_format format;
				i += get_format(&format, &str[i]);
				int width=0;
				int precision = 0;

				if(format.width == FORMAT_ARG_SPECIFIED) {
					format.width = va_arg(args, int);
				}

				if(format.precision == FORMAT_ARG_SPECIFIED) {
					format.precision = va_arg(args, int);
				}

				switch(format.type) {
					case NO_FORMAT:
						break;
					case INT:
						if(format.is_unsigned) {
							print_unsigned_int(&format, va_arg(args, unsigned int));
						} else {
							print_signed_int(&format, va_arg(args, int));
						}
						break;
					case S_CHAR:
						break;
					case CHAR:
						if(format.is_pointer) {
							print_string(&format, (char*)va_arg(args, int));
						} else {
							print_char(&format, (char)va_arg(args, int));
						}
						break;
					case SHORT_INT:
						print_short_int(&format, (short int)va_arg(args, int));
						break;
					case LONG_INT:
						print_long_int(&format, va_arg(args, long int));
						break;
					case LONG_LONG_INT:
						print_long_long_int(&format, va_arg(args, long long int));
						break;
					case INTMAX_T:
						print_intmax_t(&format, va_arg(args, intmax_t));
						break;
					case SIZE_T:
						print_size_t(&format, va_arg(args, size_t));
						break;
					case PTRDIFF_T:
						print_ptrdiff_t(&format, va_arg(args, ptrdiff_t));
						break;
					case LONG_DOUBLE:
						print_long_double(&format, va_arg(args, long double));
						break;
					case WINT_T:
						//print_wint_t(format, va_arg(args, wint_t));

						//unknown type?
						break;
					case WCHAR_T:
						print_wchar_t(&format, va_arg(args, wchar_t));
						break;
					case DOUBLE:
						print_double(&format, va_arg(args, double));
						break;
					default:
						assert(0!=0);
				}
				continue;
			}
		}

		if(str[i] == '\n') {
			print_newline();
		} else if(str[i] == '\t') {
			for(int tab_cnt=0; tab_cnt < tab_space_num; ++tab_cnt) {
				terminal_put_char(make_vga_char(' ', terminal.default_color));
			}
		} else {
			//print normal char
			terminal_put_char(make_vga_char(str[i], terminal.default_color));
		}
	}

	va_end(args);
}