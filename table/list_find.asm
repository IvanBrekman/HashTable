global list_find_asm

section .text


list_find_asm:
								            ; rdi = list* lst
		mov         rsi, QWORD [rsi]        ; rsi = char* str
		mov 	    edx, DWORD [rdi+40]     ; rdx = list->size
		mov		    rcx, QWORD [rdi+32]		; rcx = list->data
		
		mov         r8,  0			        ; cycle counter: i = 0

    check_cycle:
		cmp 	    r8,  rdx		        ; cmp(i, lst->size)
		jge		    end_cycle		        ; if (i >= lst->size) end_cycle
	
	body_cycle:
		mov 	    r9,  r8
		sal 	    r9,  3
		add		    r9,  rcx		        ; r9 = list->data + i
		mov		    r9, QWORD [r9]		    ; r9 = list->data[i]
		
		vlddqu 	    ymm0, [rsi]
		vlddqu 	    ymm1, [r9]
		vpcmpeqb 	ymm2, ymm0, ymm1
		vpmovmskb	r10,  ymm2	            ; r10 = cmp(str, list->data[i])
		
		cmp 	    r10d, 0xFFFFFFFF
		jne 	    inc_cycle		        ; if (r10 != -1 /* if not find */) continue cycle
		
		mov		    rax, r8			        ; return i
		jmp		    end_find
	
	inc_cycle:
		inc 	    r8				        ; i++
		jmp 	    check_cycle
	
	end_cycle:
		mov 	    eax, -64197		        ; return NOT_FOUND;
	
	end_find:
		ret
