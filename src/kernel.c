#include "terminal.h"
#include "multiboot.h"
#include "pic.h"
#include "keyboard.h"
#include "frame_allocator.h"
#include "paging.h"
#include "kmalloc.h"

#include "assert.h"

//Calling convention on x86-64 System V ABI
//rdi, rsi, rdx, rcx for ints

void page_fault(void) {
	terminal_printf("Fault");
	assert(0!=0);
}

void kernel_main(uintptr_t pmultiboot) {
	init_terminal();

	pic_init(0x20, 0x28);
	pic_enable_interrupts();

	keyboard_init();
	add_interrupt_handler(0x21, keyboard_interrupt);
	add_interrupt_handler(0x0e, page_fault);

	init_multiboot_data(pmultiboot);

	init_allocator(&data);

	remap_kernel();
	init_multiboot_data(pmultiboot);

	init_heap();
	//test();

	while(1) {
		// struct frame f;
		// allocate_frame(&f);

		// print_text("Alloc frame num: ");
		// print_hex_number(f.number);
		// print_newline();
		int foo = 10;
		int bar = 0xdeadbeef;
		size_t baz = 0xcafebabecafebabe;
		//terminal_printf("\ntest\ntest2\ntest3\n%x\n%3X\n%#x\n%X\n%zx", foo, foo, bar, bar, baz);

		for(int k = 0; k < 100000000; ++k) {
		}

		intptr_t addr = kmalloc(0x100);
		terminal_printf("Addr %#zX\n", addr);
	}
}