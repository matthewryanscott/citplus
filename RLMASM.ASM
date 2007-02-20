; --------------------------------------------------------------------- ;
; rlmasm.asm			Citadel 				;
;	RLM stuff that I couldn't figure out how to do in C		;
; --------------------------------------------------------------------- ;

ideal
model large, c

ifndef WINCIT
public citEntryPoint

dataseg
extrn	RLMptrs: word, citFunc: word

codeseg

; entry point to cit from an RLM
proc citEntryPoint
	mov ax, @data
	mov ds, ax

	mov bx, offset RLMptrs
	mov dx, [citFunc]
	shl dx, 1
	shl dx, 1
	add bx, dx
	db 0ffh, 2fh	; jmp far [bx], but i couln't figure out how
endp

endif
end
