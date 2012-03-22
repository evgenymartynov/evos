[GLOBAL clone_physical_page]

; TODO
; This is most likely broken as I did not test it yet.
; Will need to fix later, most likely.
; Especially parameter fetching (I think it is kinda wrong).
clone_physical_page:
    pushf
    push ecx
    push esi
    push edi

    mov ecx, 4096
    mov edi, [esp+5*4]
    mov esi, [esp+6*4]

    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax

    cld
    rep movsb

    or eax, 0x80000000
    mov cr0, eax

    pop edi
    pop esi
    pop ecx
    popf
    ret
