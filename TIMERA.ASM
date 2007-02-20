; --------------------------------------------------------------------- ;
; timera.asm		high-resolution timer stuff.			;
; --------------------------------------------------------------------- ;

ideal
model large, c

ifndef WINCIT
; --------------------------------------------------------------------- ;
;				Contents				;
; --------------------------------------------------------------------- ;
;  OurTimerHandler	Counts MSW. 					;
; --------------------------------------------------------------------- ;

public OurTimerHandler, OurTimerMSW

dataseg
extrn SystemTimerHandler: word

OurTimerMSW dw 0

codeseg
CS_STH	dd 0

proc OurTimerHandler
	push ds
	push ax
	mov ax, DGROUP
	mov ds, ax
	inc [OurTimerMSW]

	mov ax, [SystemTimerHandler]
	mov [word ptr cs:CS_STH], ax
	mov ax, [SystemTimerHandler + 2]
	mov [word ptr cs:CS_STH + 2], ax

	pop ax
	pop ds

	jmp [cs:CS_STH]
endp


endif
end
