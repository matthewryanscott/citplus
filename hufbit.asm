; --------------------------------------------------------------------- ;
; hufbit.asm			Acit				92Jun13 ;
;		bit by bit string manipulations for huffman encoding	;
; --------------------------------------------------------------------- ;

ideal
model large, c


theChar = 0
right	= 2
left	= 6
ENCBUFSIZE = 8192

struc eds
	nb	dw	?
	what	dd	?
ends

public decode

dataseg
extrn bitsenc: word, htRoot: word

codeseg

proc decode uses si di, strofs:word, strseg:word
	mov di, [htRoot]
	mov ax, [htRoot+2]
	mov es, ax
	mov si, [bitsenc]

theLoop:mov ax,[es:di + left]
	or ax,[es:di + left + 2]
	jz noMore

	push es

	mov bx, [strseg]
	mov es, bx
	mov di, [strofs]
	mov bx, si
	and bx, 7
	or bx, bx
	jnz notFirstBit

	; first bit of new byte - check for 0x27
	mov bx, si
;	shr bx, 3
	shr bx, 1
	shr bx, 1
	shr bx, 1
	mov dl, [es:di+bx]
	cmp dl, 27
	jne notFirstBit
	add si, 8
	inc bx
	mov dl, [es:di+bx]
	cmp dl, 1
	jne not1
	; got 27 1 sequence - make it 0
	mov [byte ptr es:di+bx], 0
	jmp short notFirstBit

not1:	cmp dl, 2
	jne not2
	; got 27 2 sequence - make it 27
	mov [byte ptr es:di+bx], 27
	jmp short notFirstBit

not2:	; assume 27 3 sequence - make it 255
	mov [byte ptr es:di+bx], 255

notFirstBit:
	mov bx, si
;	shr bx, 3
	shr bx, 1
	shr bx, 1
	shr bx, 1
	mov dl, [es:di+bx]

	mov bx, si
	and bx, 7
	mov cl, 7
	sub cl, bl
	mov bl, 1
	shl bl, cl
	and dl, bl

	inc si
	mov al, dl

	pop es
	or al, al
	jz goLeft

; go right
	mov ax, [es:di + right + 2]
	mov di, [es:di + right]
	mov es, ax
	jmp short theLoop

goLeft: mov ax, [es:di + left + 2]
	mov di, [es:di + left]
	mov es, ax
	jmp theLoop

noMore: mov al, [es:di + theChar]
	cbw
	mov [bitsenc], si
	ret
endp

end
