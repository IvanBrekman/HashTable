global table_find

section .text

extern list_find_asm

table_find:
                                            ; rdi = HashTable* table
                                            ; rsi = char** str

        ; ========== HASH FUNC ========== ;
        xor         rax, rax                ; hash = 0
        mov         r9,  QWORD [rsi]        ; r9   = char* str

        mov         rcx, 4

        calc_hash:
            mov     r8,  [r9]

            crc32   rax,  r8
            add     r9,  8
            loop    calc_hash
        ; =============================== ; ; rax  = hash

        xor         edx, edx

        mov         r10d, DWORD [rdi+68]    ; r10 = table->capacity
        div         r10d                    ; edx = hash % table->capacity
        
        mov			r8,  rdi				; r8  = HashTable* table;
        
        lea			rdi, [rdx+rdx*2]		; rdi = hash * 48 (sizeof List);
        sal			rdi, 4
        add			rdi, QWORD [r8+56]		; rdi = hash + table->data;
                                            ; rsi = char** str
        jmp         list_find_asm			; return list_find(table->data + hash, item);
