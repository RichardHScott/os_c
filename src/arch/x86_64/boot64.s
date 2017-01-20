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

setup_interrupt_handlers:
    ;mov qword [interrupt_descriptor_table.base], interrupt_vectors

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

    ;lea rax, interrupt_descriptor_table
    ;mov qword [interrupt_descriptor_table.pointer], interrupt_descriptor_table
    ;mov eax, interrupt_descriptor_table
    ;mov dword [interrupt_descriptor_table.pointer], eax
    lidt [interrupt_descriptor_table]
    
    mov r8, 0

    ret

i_keyboard:
    pushad
;    mov rdi, "K"
;    call print_char

    
    in al, 0x60
    mov dl, al
    call print_char

    mov al, 0x20
    out 0x20, al

    popad
    iretq   

i_ok_timer:
    pushad

    mov rdi, "T"
    call print_char
    pop rdi

    mov al, 0x20
    out 0x20, al
    
    popad

    iretq

i_ok:
;    mov qword [0xb8012], r8
;    cmp r8, 1
;    je .reset
;    mov r8, 1
    pushad
    mov rdi, "N"
    call print_char

    mov dword [0xb8000], 0x2f522f45
    mov dword [0xb8004], 0x2f3a2f52
    mov dword [0xb8008], 0x2f202f20
    jmp .done
.reset:
    mov r8, 0
    mov dword [0xb8000], 0x2f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
.done:
    mov al, 0x20
    out 0x20, al

    popad

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