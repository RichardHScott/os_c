#include "terminal.h"
#include "multiboot.h"
#include "pic.h"

//Calling convention on x86-64 System V ABI
//rdi, rsi, rdx, rcx for ints

void kernel_main(uintptr_t pmultiboot) {
	init_terminal();

	pic_init(0x20, 0x28);

	print_text("\nMultiboot data address: ");
	print_hex_number(pmultiboot);
	print_newline();
	init_multiboot_data(pmultiboot);

	for(int k=0; k<60;++k) {
		for(int i=0; i < 50000000; ++i) { }
	// print_text("\nNope!");
	// print_hex_number(k);
	// print_text("Hi1!");
	}
}