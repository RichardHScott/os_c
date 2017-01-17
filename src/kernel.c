#include "terminal.h"

void kernel_main(void) {
	init_terminal();

	for(int k=0; k<60;++k) {
		for(int i=0; i < 50000000; ++i) { }
	print_text("\nNope!");
	print_hex_number(k);
	print_text("Hi1!");
	}
}