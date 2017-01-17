global long_mode_start
extern kernel_main

section .text
bits 64

long_mode_start:
    call ok

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