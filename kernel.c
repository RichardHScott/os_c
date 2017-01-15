#include "terminal.h"

void kernel_main(void) {
	init_terminal();

	for(int k=0; k<30;++k) {
		for(int i=0; i < 100000000; ++i) { }
	print_text("\nNope!");
	print_text((char*) (k+060));
	print_text("Hi1!");
	}
}