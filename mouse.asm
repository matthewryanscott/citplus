; Citadel: Mouse.Asm
;
; mouse handling stuff

ifndef WINCIT

ideal
model large, c

public initMouseHandler, deinitMouseHandler, showMouse, hideMouse
public setMousePos, hideCounter

dataseg
extrn	mouseFlags: word, mouseX: word, mouseY: word, mouseButtons: word
extrn	mouseEvent: word

hideCounter dw 0

codeseg

proc showMouse
	dec [hideCounter]
	mov ax, 1
	int 33h
	ret
endp

proc hideMouse
	inc [hideCounter]
	mov ax, 2
	int 33h
	ret
endp

proc mouseHandler
	push ds
	push ax

	mov ax, @data
	mov ds, ax

	pop ax
	mov [mouseFlags], ax
	mov [mouseButtons], bx
	mov [mouseX], cx
	mov [mouseY], dx
	mov [mouseEvent], 1

	pop ds
	ret
endp

proc initMouseHandler
	xor ax, ax
	int 33h

	mov ax, 000ch
	mov cx, 007fh
	mov dx, cs
	mov es, dx
	mov dx, offset mouseHandler
	int 33h

	ret
endp

proc deinitMouseHandler
	xor ax, ax
	int 33h
	ret
endp

proc setMousePos xpos : word, ypos : word
	mov ax, 4
	mov cx, [xpos]
	mov dx, [ypos]
	int 33h
	ret
endp

endif
end
