; skeleton kbd driver for citadel

ideal
model large, c

codeseg

org 0
desc	db 'Keyboard Flip 1.00'
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

; -----------------------------------------------------------------------------
proc stat_kbd
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
	mov	ah, 0			; read keyboard character
        int     16h                     ; BIOS keyboard service

        or      al, al                  ; extended character?
        je      Extended                
        xor     ah, ah                  ; no, zero high byte

	cmp	al, 'z'
	ja	Extended

	cmp	al, 'a'
	jae	is_lower

	cmp	al, 'Z'
	ja	Extended

	cmp	al, 'A'
	jb	Extended

	add	al, 64

is_lower:
	sub	al, 32

Extended:
        ret
endp

end
