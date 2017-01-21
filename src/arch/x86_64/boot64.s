global long_mode_start
global asm_add_interrupt_handler

extern kernel_main
extern print_char
extern print_hex_uint64
extern print_newline

global i_ok

section .text
bits 64

long_mode_start:
    call ok

    call setup_interrupt_handlers

    call kernel_main

    mov dword [0xb8000], 0x4f204f20
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x2f524f45

    hlt

ok:
    ; print `OK` to screen
    mov dword [0xb8000], 0x2f4b2f4f
    ret

error:
    ;print 'ERR: ' to screen
    ;followed by error code in al (which is in ascii)
    mov dword [0xb8000], 0x2f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte [0xb800c], al
    hlt

done:


; rax - register a extended
; rbx - register b extended
; rcx - register c extended
; rdx - register d extended
; rbp - register base pointer (start of stack)
; rsp - register stack pointer (current location in stack, growing downwards)
; rsi - register source index (source for data copies)
; rdi - register destination index (destination for data copies)
; r8 - register 8
; r9 - register 9
; r10 - register 10
; r11 - register 11
; r12 - register 12
; r13 - register 13
; r14 - register 14
; r15 - register 15

; Callee will save RBP, RBX, and R12–R15
; We save the other registers
; Shouldn't be touching the XMM registers, but check this!
; GCC might be using them
%macro save_registers 0
    sub rsp, 130
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro restore_registers 0
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    add rsp, 130
%endmacro

%macro interrupt_shim_x 1
interrupt_shim_%1:
    save_registers
    cld

    call [interrupt_functions + %1 * 8]

    mov al, 0x20
    out 0x20, al

    restore_registers

    iretq
%endmacro

%assign interrupt_shim_counter 0
%rep 256
    interrupt_shim_x interrupt_shim_counter
%assign interrupt_shim_counter interrupt_shim_counter+1
%endrep

;function prototype asm_add_interrupt_handler(uint16_t interrupt, void(*handler)(void))
;x86_64 system V ABI has rsi = remapped interrupt number (e.g. timer = 0x20, keyboard = 0x21), rdi = handler
asm_add_interrupt_handler:
    mov qword [interrupt_functions + edi*8], rsi
    ret

;interrupt address in rax
;interrupt vector in rbx
create_interrupt:
    imul rbx, 16               ; interrupt table addr = interrupt vector * sizeof(interrupt struct) 
    add rbx, interrupt_vectors ;                        + base_interrupt_addr_offset

    mov word [rbx], ax
    mov word [rbx+2], 8
    mov word [rbx+4], 0x8e00

    shr rax, 16
    mov word [rbx+6], ax

    shr rax, 16
    mov dword [rbx+8], eax
    mov dword [rbx+12], 0

    ret

setup_interrupt_handlers:
    mov rcx, 0
.setup_idt_loop:
    mov rax, i_ok
    cmp rcx, 0x20
    jnz .not_timer
    mov rax, i_ok_timer
.not_timer:

    ; rax = interrupt function
    mov qword [rcx*8 + interrupt_functions], rax
    mov rax, [interrupt_shim_addr_base + rcx*8]
    mov rbx, rcx
    call create_interrupt

    inc rcx
    cmp rcx, 256
    jne .setup_idt_loop

    lidt [interrupt_descriptor_table]
    
    mov r8, 0

    ret

i_ok_timer:
    ret

i_ok:

    mov al, 0x20
    out 0x20, al
    ret

section .bss
align 8
;8byte ptrs to interrupt functions
interrupt_functions:
    resb 256*8

;the interrupt vector is a shim around pop/save registers
interrupt_vectors:
    resb 256*16

section .rodata
align 4
interrupt_descriptor_table:
.limit:
    dw 16*256-1
.base:
    dq interrupt_vectors
.pointer:
    dq interrupt_descriptor_table

%macro get_interrupt_function_addr_x 1
    dq interrupt_shim_%1
%endmacro

align 8
interrupt_shim_addr_base:
%assign interrupt_shim_counter 0
%rep 256
    get_interrupt_function_addr_x interrupt_shim_counter
%assign interrupt_shim_counter interrupt_shim_counter+1
%endrep