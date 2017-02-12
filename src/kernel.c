#include "terminal.h"
#include "multiboot.h"
#include "pic.h"
#include "keyboard.h"
#include "frame_allocator.h"
#include "paging.h"
#include "kmalloc.h"

#include "exceptions.h"

#include "assert.h"


//Calling convention on x86-64 System V ABI
//rdi, rsi, rdx, rcx for ints

static void loop(void) {
	for(int k = 0; k < 100000000; ++k) {}
}

static void loop_small(void) {
	for(int k = 0; k < 50000000; ++k) {}
}

static void pf_test(int i) {
	pf_test(++i);
}

void kernel_main(uintptr_t pmultiboot) {
	init_terminal();

	pic_init(0x20, 0x28);
	pic_enable_interrupts();

	init_exception_handlers();

	keyboard_init();
	add_interrupt_handler(0x21, keyboard_interrupt);

	init_multiboot_data(pmultiboot);

	init_allocator(&data);

	remap_kernel();

	init_heap();

	//int b = 0/0;
	//*(int*)(0xdeadb00) = 20;
	asm volatile("int $3");
		asm volatile("int $3");
			asm volatile("int $3");

	//test();

	// intptr_t addr1 = kmalloc(0x100);
	// terminal_printf("Addr1 %#zX\n", addr1);
	// loop_small();
	// intptr_t addr2 = kmalloc(0x200);
	// terminal_printf("Addr2 %#zX\n", addr2);
	// loop_small();
	// intptr_t addr3 = kmalloc(0x100);
	// terminal_printf("Addr3 %#zX\n", addr3);
	// kfree(addr2);
	// intptr_t addr = kmalloc(0x20);
	// terminal_printf("Addr4 %#zX\n", addr);
	// loop_small();
	// addr = kmalloc(0x40);
	// terminal_printf("Addr5 %#zX\n", addr);
	// loop_small();
	// addr = kmalloc(0x50);
	// terminal_printf("Addr6 %#zX\n", addr);
	// loop_small();


	int ptr_cnt = 0;
	intptr_t ptrs[10];
	
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

		loop();

		// intptr_t addr = kmalloc(0x10*(1+ptr_cnt));
		// terminal_printf("Addr %#zX\n", addr);
		// if(ptr_cnt < 10) {
		// 	ptrs[ptr_cnt++] = addr;
		// } else {
		// 	for(int i=0; i < 10; ++i) {
		// 		kfree(ptrs[i]);
		// 	}
		// 	ptr_cnt = 0;
		// }
	}
}