global table_find

section .text

extern list_find_asm

table_find:
                                            ; rdi = HashTable* table
                                            ; rsi = char** str

        ; ========== HASH FUNC ========== ;
        mov         r8,  0                  ; hash = 0
        mov         r9,  QWORD [rsi]        ; r9   = char* str

        mov         rcx, 4

        calc_hash:
            mov     rax, [r9]

            crc32   r8,  rax
            add     r9,  8
            loop    calc_hash
        ; =============================== ; ; r8   = hash

        mov         rax, r8
        mov         edx, 0

        mov         r10d, DWORD [rdi+68]    ; r10 = table->capacity
        div         r10d                    ; edx = hash % table->capacity

        mov         rdi, QWORD [rdi+56]     ; rdi = table->data
        mov         rax, rdx                ;   ------------------------+
        add         rdx, rdx                ;                           |
        add         rdx, rax                ;                           |
        sal         rdx, 4                  ; rdx *= 48 (sizeof List) <-+
        add         rdi, rdx                ; rdi = table->data + hash
                                            ; rsi = char** str
        call        list_find_asm
        ret                                 ; return list_find(table->data + hash, item);
