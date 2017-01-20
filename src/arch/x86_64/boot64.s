global long_mode_start
extern kernel_main
extern print_char

section .text
bits 64

long_mode_start:
    call ok

    call setup_interrupt_handlers

    call kernel_main

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
%endmacro

setup_interrupt_handlers:
    mov rcx, 0
.setup_idt_loop:
    mov rax, i_ok
    cmp rcx, 0x20
    jnz .not_timer
    mov rax, i_ok_timer
.not_timer:
    cmp rcx, 0x21
    jnz .not_keyboard
    mov rax, i_keyboard
.not_keyboard:
    mov rbx, rcx
    imul rbx, 16
    add rbx, interrupt_vectors

    mov word [rbx], ax
    mov word [rbx+2], 8
    mov word [rbx+4], 0x8e00

    shr rax, 16
    mov word [rbx+6], ax

    shr rax, 16
    mov dword [rbx+8], eax
    mov dword [rbx+12], 0

    inc rcx
    cmp rcx, 256
    jne .setup_idt_loop

    lidt [interrupt_descriptor_table]
    
    mov r8, 0

    ret

i_keyboard:
    save_registers

    in al, 0x60
    mov edi, 0x2f002f00
    or dl, al
    mov dword [0xb8000], edi
    call print_char

    mov al, 0x20
    out 0x20, al

    restore_registers
    iretq   

i_ok_timer:
    ;call save_registers
    push rdi
    push rax
    mov rdi, "T"
    call print_char

    mov al, 0x20
    out 0x20, al
    pop rax
    pop rdi
    ;call restore_registers
    iretq

i_ok:
    save_registers
    ;mov rdi, "N"
    ;call print_char

    mov dword [0xb8000], 0x2f522f45
    mov dword [0xb8004], 0x2f3a2f52
    mov dword [0xb8008], 0x2f202f20

    mov al, 0x20
    out 0x20, al

    restore_registers

    iretq

align 4
interrupt_vectors:
    resb 256*16
interrupt_descriptor_table:
.limit:
    dw 16*256-1
.base:
    dq interrupt_vectors
.pointer:
    dq interrupt_descriptor_table