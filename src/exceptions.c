#include "exceptions.h"

struct exception_info {
    intptr_t instruction_ptr;
    intptr_t code_segement;
    uintptr_t flags;
    intptr_t stack_ptr;
    intptr_t stack_segement;
} __attribute__ ((packed));

struct exception_info_with_error {
    uintptr_t error_code;
    intptr_t instruction_ptr;
    intptr_t code_segement;
    uintptr_t flags;
    intptr_t stack_ptr;
    intptr_t stack_segement;
} __attribute__ ((packed));

/*
Divide-by-zero Error	0 (0x0)	Fault	#DE	No
Debug	1 (0x1)	Fault/Trap	#DB	No
Non-maskable Interrupt	2 (0x2)	Interrupt	-	No
Breakpoint	3 (0x3)	Trap	#BP	No
Overflow	4 (0x4)	Trap	#OF	No
Bound Range Exceeded	5 (0x5)	Fault	#BR	No
Invalid Opcode	6 (0x6)	Fault	#UD	No
Device Not Available	7 (0x7)	Fault	#NM	No
Double Fault	8 (0x8)	Abort	#DF	Yes (Zero)
Coprocessor Segment Overrun	9 (0x9)	Fault	-	No
Invalid TSS	10 (0xA)	Fault	#TS	Yes
Segment Not Present	11 (0xB)	Fault	#NP	Yes
Stack-Segment Fault	12 (0xC)	Fault	#SS	Yes
General Protection Fault	13 (0xD)	Fault	#GP	Yes
Page Fault	14 (0xE)	Fault	#PF	Yes
Reserved	15 (0xF)	-	-	No
x87 Floating-Point Exception	16 (0x10)	Fault	#MF	No
Alignment Check	17 (0x11)	Fault	#AC	Yes
Machine Check	18 (0x12)	Abort	#MC	No
SIMD Floating-Point Exception	19 (0x13)	Fault	#XM/#XF	No
Virtualization Exception	20 (0x14)	Fault	#VE	No
Reserved	21-29 (0x15-0x1D)	-	-	No
Security Exception	30 (0x1E)	-	#SX	Yes
Reserved	31 (0x1F)	-	-	No
Triple Fault	-	-	-	No
FPU Error Interrupt	IRQ 13	Interrupt	#FERR	No
*/

enum exception_id {
    ex_divide_by_zero = 0,
    ex_debug = 1,
    ex_breakpont = 3,
    ex_overflow = 4,
    ex_bound_range_exceeded = 5,
    ex_invalid_opcode = 6,
    ex_device_not_available = 7,
    ex_double_fault = 8,
    ex_coprocessor_segement_overrun = 9,
    ex_invalid_tss = 0xa,
    ex_segement_not_present = 0xb,
    ex_stack_segement_fault = 0xc,
    ex_general_protection_fault = 0xd,
    ex_page_fault = 0xe,
    ex_x87_fpu_exception = 0x10,
    ex_alignment_check = 0x11,
    ex_machine_check = 0x12,
    ex_simd_fp = 0x13,
    ex_vx = 0x14,
    ex_security = 0x1e
};

const char* exception_id_strings[] = {
    "divide_by_zero",
    "debug",
    "breakpont",
    "overflow",
    "bound_range_exceeded",
    "invalid_opcode",
    "device_not_available",
    "double_fault",
    "coprocessor_segement_overrun",
    "invalid_tss",
    "segement_not_present",
    "stack_segement_fault",
    "general_protection_fault",
    "page_fault",
    "x87_fpu_exception",
    "alignment_check",
    "machine_check",
    "simd_fp",
    "vx",
    "security"
};

static int divide_by_zero_handler(struct exception_info* info){
    asm volatile("mov %0, %%eax"
                 : 
                 : ""(ex_divide_by_zero)
                 : "%eax");

    terminal_printf(exception_id_strings[ex_divide_by_zero]);
    terminal_printf("iptr: %#zX \t code_seg: %#zX \t flags: %#zX\n", info->instruction_ptr, info->code_segement, info->flags);
    terminal_printf("Stackptr: %#zX \t Stackseg: %#zX\n", info->stack_ptr, info->stack_segement);

    return 0;
    asm volatile("hlt");
}

static int debug_handler(struct exception_info* info){
    asm volatile("mov %0, %%eax"
                 : 
                 : ""(ex_debug)
                 : "%eax");

    terminal_printf(exception_id_strings[ex_debug]);
    terminal_printf("iptr: %#zX \t code_seg: %#zX \t flags: %#zX\n", info->instruction_ptr, info->code_segement, info->flags);
    terminal_printf("Stackptr: %#zX \t Stackseg: %#zX\n", info->stack_ptr, info->stack_segement);

    return 0;
}

static int breakpont_handler(struct exception_info* info){
    asm volatile("mov %0, %%eax"
                 : 
                 : ""(ex_breakpont)
                 : "%eax");

    terminal_printf(exception_id_strings[ex_breakpont]);
    terminal_printf("iptr: %#zX \t code_seg: %#zX \t flags: %#zX\n", info->instruction_ptr, info->code_segement, info->flags);
    terminal_printf("Stackptr: %#zX \t Stackseg: %#zX\n", info->stack_ptr, info->stack_segement);

    return 0;
}

static void overflow_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void bound_range_exceeded_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void invalid_opcode_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void device_not_available_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void double_fault_handler(struct exception_info* info){
    asm volatile("mov %0, %%eax"
                 : 
                 : ""(ex_double_fault)
                 : "%eax");

    terminal_printf(exception_id_strings[ex_double_fault]);
    terminal_printf("iptr: %#zX \t code_seg: %#zX \t flags: %#zX\n", info->instruction_ptr, info->code_segement, info->flags);
    terminal_printf("Stackptr: %#zX \t Stackseg: %#zX\n", info->stack_ptr, info->stack_segement);

    asm volatile("hlt");
}

static void coprocessor_segement_overrun_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void invalid_tss_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void segement_not_present_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void stack_segement_fault_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void general_protection_fault_handler(struct exception_info* info){
    asm volatile("mov %0, %%eax\nhlt"
                 : 
                 : ""(ex_general_protection_fault)
                 : "%eax");
}

enum page_fault_error_code {
    page_fault_present = 1,
    page_fault_write = 1 << 1,
    page_fault_user = 1 << 2,
    page_fault_reserved_write = 1 << 3,
    page_fault_instruction_fetch = 1 << 4
};

static int page_fault_handler(struct exception_info_with_error* info){
    asm volatile("mov %0, %%eax\n"
                 : 
                 : ""(ex_page_fault)
                 : "%eax");
    
    terminal_printf(exception_id_strings[ex_page_fault]);
    terminal_printf("iptr: %#zX \t code_seg: %#zX \t flags: %#zX\n", info->instruction_ptr, info->code_segement, info->flags);
    terminal_printf("Stackptr: %#zX \t Stackseg: %#zX\n", info->stack_ptr, info->stack_segement);
    terminal_printf("Error code: %#zX\n", info->error_code);

    //return 1;
    asm volatile("hlt");
}

static void x87_fpu_exception_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void alignment_check_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void machine_check_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void simd_fp_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void vx_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

static void security_handler(struct exception_info* info){
    asm volatile(""
                 :
                 :
                 :);
}

void init_exception_handlers(void) {
    add_interrupt_handler(ex_divide_by_zero, divide_by_zero_handler);
    add_interrupt_handler(ex_debug, debug_handler);
    add_interrupt_handler(ex_breakpont, breakpont_handler);
    add_interrupt_handler(ex_overflow, overflow_handler);
    add_interrupt_handler(ex_bound_range_exceeded, bound_range_exceeded_handler);
    add_interrupt_handler(ex_invalid_opcode, invalid_opcode_handler);
    add_interrupt_handler(ex_device_not_available, device_not_available_handler);
    add_interrupt_handler(ex_double_fault, double_fault_handler);
    add_interrupt_handler(ex_coprocessor_segement_overrun, coprocessor_segement_overrun_handler);
    add_interrupt_handler(ex_invalid_tss, invalid_tss_handler);
    add_interrupt_handler(ex_segement_not_present, segement_not_present_handler);
    add_interrupt_handler(ex_stack_segement_fault, stack_segement_fault_handler);
    add_interrupt_handler(ex_general_protection_fault, general_protection_fault_handler);
    add_interrupt_handler(ex_page_fault, page_fault_handler);
    add_interrupt_handler(ex_x87_fpu_exception, x87_fpu_exception_handler);
    add_interrupt_handler(ex_alignment_check, alignment_check_handler);
    add_interrupt_handler(ex_machine_check, machine_check_handler);
    add_interrupt_handler(ex_simd_fp, simd_fp_handler);
    add_interrupt_handler(ex_vx, vx_handler);
    add_interrupt_handler(ex_security, security_handler);
}