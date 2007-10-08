; skeleton kbd driver for citadel

ideal
model large, c

codeseg

org 0
desc	db 'Macro Keyboard 1.00'
	db 32 - ((offset $ - offset desc)) dup(0)
				; filler for alignment

	dw offset init_kbd	; open kbd
	dw offset close_kbd	; close kbd

	dw offset stat_kbd	; are there keys waiting?
	dw offset get_kbd	; get key

	dw 124 dup (0)		; 128 total entries reserved


; -----------------------------------------------------------------------------
proc init_kbd
	ret
endp

; -----------------------------------------------------------------------------
proc close_kbd
	ret
endp

mCnt	db	0
m	db	'this is wow and don''t you forget it! '
ml	=	$ - m

; -----------------------------------------------------------------------------
proc stat_kbd
	cmp	[cs:mCnt], 0
	jne	Char

	mov	ah, 1			; read keyboard status
        int     16h                     ; BIOS keyboard service

	jnz	Char			; is there a character?

	xor	ax, ax			; no character, return zero
        ret

Char:	mov	ax, 1			; character, return non zero
        ret
endp

; -----------------------------------------------------------------------------
proc get_kbd
	cmp	[cs:mCnt], 0
	jne	in_m

	mov	ah, 0			; read keyboard character
        int     16h                     ; BIOS keyboard service

        or      al, al                  ; extended character?
        je      Extended                
        xor     ah, ah                  ; no, zero high byte
not_m:	ret

Extended:
	cmp	ah, 50
	jne	not_m
	mov	[cs:mCnt], ml

in_m:
	mov	bx, ml
	sub	bl, [cs:mCnt]
	sbb	bh, 0
	mov	al, [cs:m + bx]
	xor	ah, ah
	dec	[cs:mCnt]
	ret
endp

end
