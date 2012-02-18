[GLOBAL gdt_load]

gdt_load:
    mov eax, [esp+4]
    lgdt [eax]

    ; Fix up the data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 0x08 is the new CS (gdt_entries[1], and sizeof is 8)
    ; So we do a far-jump
    jmp 0x08:.next
.next:
    ret
