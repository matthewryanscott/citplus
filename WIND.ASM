; --------------------------------------------------------------------- ;
; wind.asm			Citadel 				;
;			fast stuff for screen i/o			;
; --------------------------------------------------------------------- ;

ideal
model large, c

; --------------------------------------------------------------------- ;
;				Contents				;
; --------------------------------------------------------------------- ;
;  cls_fast		Clears the screen				;
;  physPosition		Positions the cursor				;
;  clearline		Clears a line on the screen			;
;  physReadpos		Reads the cursor position			;
;  readcursorsize	Reads the cursor size				;
;  fastcpy		Copies memory quickly				;
;  directstring		Writes string to screen				;
;  directchar		Writes character to screen			;
;  fastsetw		Like memset() but uses words, not chars		;
;  statDisp		Displsy for F4 screen				;
;  gmode		Returns current video mode			;
;  curshalf		Sets the cursor to be a half-block		;
;  curson		Turns on display of the cursor			;
;  cursoff		Turns off display of the cursor			;
;  putatrb		Sets attribute of an area of the screen		;
;  putmsgnc		Displays on the screen without changing color	;
;  critical_error	Tells DOS what to do				;
; --------------------------------------------------------------------- ;

public cls_fast, physPosition, physReadpos, fastcpy, clearline, directchar
public directstring, fastsetw, statDisp, gmode, curson, cursoff
public curshalf, critical_error
; public putatrb, putmsgnc

dataseg
extrn	dgLogiScreen: word, physScreen: word, conRows: byte, conCols: byte
extrn	conMode: word, dgLogiRow: byte, dgLogiCol: byte

codeseg

proc cls_fast uses di, attr:byte
	mov di, [dgLogiScreen]
	mov es, [dgLogiScreen+2]

	mov al, [conCols]
	mov bl, [conRows]
	inc bl
	mul bl

	mov cx, ax

	xor ax, ax
	mov ah, [attr]

	rep stosw
	ret
endp

proc physPosition row:byte, col:byte
	mov ah, 2
	mov dh, [row]
	mov dl, [col]
	mov bh, 0
	int 10h
	ret
endp

proc clearline row:byte, attr:byte
	mov ah, 2
	xor dx, dx
	mov dh, [row]
	mov bh, 0
	int 10h

	mov ax, 0920h		; 09h - write; 20h - space
	mov bl, [attr]
	xor cx, cx
	mov cl, [conCols]
	int 10h 		; bh should still be 0

	ret
endp

proc physReadpos row:dword, col:dword
	mov ah, 3
	mov bh, 0
	int 10h

	mov es, [word ptr row+2]
	mov bx, [word ptr row]
	mov [es:bx], dh

	mov es, [word ptr col+2]
	mov bx, [word ptr col]
	mov [es:bx], dl

	ret
endp

proc readcursorsize cstart:dword, cend:word
	mov ah, 3
	mov bh, 0
	int 10h

	mov es, [word ptr cstart+2]
	mov bx, [word ptr cstart]
	mov [es:bx], ch

	mov es, [word ptr cend+2]
	mov bx, [word ptr cend]
	mov [es:bx], cl

	ret
endp

proc fastcpy uses ds si di, dest:dword, src:dword, len:word
	mov ds, [word ptr src+2]
	mov si, [word ptr src]

	mov es, [word ptr dest+2]
	mov di, [word ptr dest]

	mov cx, [len]
	shr cx, 1

	rep movsw

	jnc done_fastcpy

	movsb

done_fastcpy:
	ret
endp

proc directstring uses ds si di, row:word, str:dword, attr:byte, phys:byte
	cmp [phys], 0
	je ds_logi

	mov di, [physScreen]
	mov es, [physScreen+2]
	jmp short ds_out

ds_logi:
	mov di, [dgLogiScreen]
	mov es, [dgLogiScreen+2]

ds_out:
	mov al, [conCols]
	mul [byte ptr row]	; was [row]
	shl ax, 1		; where on screen...

	add di, ax

	mov ds, [word ptr str+2]
	mov si, [word ptr str]

	mov ah, [attr]

ds_loop:
	lodsb
	or al, al
	jz ds_done
	stosw
	jmp short ds_loop

ds_done:
	ret
endp


proc directchar uses di, och:byte, attr:byte, phys:byte
	cmp [phys], 0
	je dc_logi

	mov di, [physScreen]
	mov es, [physScreen+2]
	jmp short dc_out

dc_logi:
	mov di, [dgLogiScreen]
	mov es, [dgLogiScreen+2]

dc_out:
	mov dl, [dgLogiCol]
	mov dh, [dgLogiRow]
	mov cx, dx

	mov al, [conCols]
	mul dh
	mov dh, 0
	shl ax, 1
	shl dx, 1

	add di, ax
	add di, dx

	mov al, [och]
	mov ah, [attr]

	stosw

	inc cx
        cmp cl, [conCols]
	je dc_scroll

;	mov [savePos], cx

;------------------
;	mov ah, 2
;	mov dx, cx
;	mov bh, 0
;	int 10h
;------------------cursor positioning now done by caller

	xor ax, ax
	ret

dc_scroll:
	mov ax, 1
	ret
endp


proc fastsetw uses di, dest:dword, what:word, len:word
	mov es, [word ptr dest+2]
	mov di, [word ptr dest]

	mov cx, [len]
	mov ax, [what]

	rep stosw
	ret
endp


; statDisp is an ultra-high speed output routine. this is because the
; status screen is updated once a second when waiting for user input, and
; between every character entered. this needs to be a quick update to be
; invisible to a user when the bbs is running on an XT. This gets most of
; its speed by not touching the cursor position, reading or writing. It
; also is faster than a directchar loop because there is only one call
; for each string, and the output is always directed to the phsyScreen,
; never the dgLogiScreen. Therefore this routine is limited in application,
; but well worth the few bytes of .EXE
proc statDisp uses ds di si, row:word, col:word, attr:word, str:dword
	mov di, [physScreen]
	mov es, [physScreen+2]

	mov al, [conCols]
	mul [byte ptr row]
	shl ax, 1
	shl [col], 1

	add di, ax
	add di, [col]

	mov ds, [word ptr str+2]
	mov si, [word ptr str]

	mov ah, [byte ptr attr]

sd_loop:
	lodsb
	or al, al
	jz sd_done
	stosw
	jmp short sd_loop

sd_done:
	ret
endp

proc gmode
	mov ah, 0fh
	mov bh, 0
	xor cx, cx
	int 10h
	mov ah, 0
	ret
endp

proc curshalf
	call gmode

	cmp ax, 7
	jne ch_color

	cmp [conMode], 1002
	je ch_color

	mov cx, 060dh
	jmp short ch_set

ch_color:
	mov cx, 0307h

ch_set: mov ah, 1
	mov bh, 0
	int 10h
	ret
endp


proc curson
	call gmode

	cmp ax, 7
	jne co_color

	cmp [conMode], 1002
	je co_color

	mov cx, 0c0dh
	jmp short co_set

co_color:
	mov cx, 0607h

co_set: mov ah, 1
	mov bh, 0
	int 10h
	ret
endp

proc cursoff
	mov ah, 1
	mov bh, 0
	mov cx, 2607h
	int 10h
	ret
endp

;proc putatrb uses di es, ccol:word, crow:word, len:word, clr:byte
;	cld
;	mov di, [dgLogiScreen]
;	mov es, [dgLogiScreen+2]
;	mov bl, [conCols]
;
;	mov ax, [crow]
;	mul bl
;	add ax, [ccol]
;	shl ax, 1
;	add di, ax
;
;	mov cx, [len]
;	mov al, [clr]
;
;pa_lp:	inc di
;	stosb
;	loop pa_lp
;
;	ret
;endp


;proc putmsgnc uses ds di si, row:word, col:word, str:dword
;	cld
;
;	mov di, [physScreen]
;	mov es, [physScreen+2]
;
;	mov al, [conCols]
;	mul [byte ptr row]
;	shl ax, 1
;	shl [col], 1
;
;	add di, ax
;	add di, [col]
;
;	mov ds, [word ptr str+2]
;	mov si, [word ptr str]
;
;pm_loop:lodsb
;	or al, al
;	jz pm_done
;	stosb
;	inc di
;	jmp short pm_loop
;
;pm_done:
;	ret
;endp

proc critical_error
	mov al, 3	; fail error
	iret
endp

end
