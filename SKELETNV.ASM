; skeleton vid driver for citadel

ideal
model large, c

codeseg

org 0
desc	db 'Skeleton Video Driver'
	db 32 - ((offset $ - offset desc)) dup(0)
				; filler for alignment

	dw offset init_vid	; open vid
	dw offset close_vid	; close vid


	dw 123 dup (0)		; 128 total entries reserved


; -----------------------------------------------------------------------------
proc init_kbd
	ret
endp

; -----------------------------------------------------------------------------
proc close_kbd
	ret
endp


end
