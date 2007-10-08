; --------------------------------------------------------------------- ;
; auxmem.asm			Acit				92Apr21 ;
;		asm support for ems/xms/virtual memory			;
; ---------------------------------------------------------------------	;

; --------------------------------------------------------------------- ;
; these routines support msgtab.c. they implement the low-level link to ;
; xms/ems/virtual memory that would be goofy to do in c. written for	;
; borland's tasm 2.0.                                                   ;
;									;
; BiB/92Apr21								;
; ----------------------------------------------------------------------;

; --------------------------------------------------------------------- ;
;				contents				;
;									;
; setUpAuxMem() 	inits memory for our use			;
; _addToEMS()		adds another chunk to our ems pool		;
; _addToVirt()		adds another chunk to our virtual pool		;
; _addToXMS()		adds another chunk to our xms pool		;
; disposeAuxMem()	releases any allocated memory			;
; HeapToXMS()		transfers memory from Heap to XMS		;
; HeapToEMS()		transfers memory from Heap to EMS		;
; HeapToVirt()		transfers memory from Heap to Virtual		;
; EMStoHeap()		transfers memory from EMS to Heap		;
; VirttoHeap()		transfers memory from Virtual to Heap		;
; XMStoHeap()		transfers memory from XMS to Heap		;
; --------------------------------------------------------------------- ;

ideal
model large, c

ifdef AUXMEM

public setUpAuxMem, _addToXMS, _addToEMS, _addToVirt, disposeAuxMem, XMStoHeap
public EMStoHeap, VirttoHeap, HeapToXMS, HeapToEMS, HeapToVirt

public usexms, xmshand, xmssize, emsframe, emshand, emssize, vrthand
public vrtpags, xmserr, emserr
dataseg

SIZE_K		equ	4		; Size of our pages in K
SIZE_B		equ	4096		; Size of our pages in bytes
PER_EMS_PAGE	equ	4		; Number of our pages that fit in one
					; EMS page (16K) Don't be fooled:
					; Some bit-shifting goes on, and
					; code needs to be changed to change
					; this.

xmsaddr dd      0
usexms	db	0
xmshand dw	0
xmssize dw	0	; in Kbytes
xmserr	dw	0

xmsxfr	dd	SIZE_B	; for xms func 0bh
srchand dw	0
srcofs	dd	0
dsthand dw	0
dstofs	dd	0


emmname db	'EMMXXXX0'
emsframe dw	0
emshand dw	0
emssize dw	0	; in our pages (SIZE_K)
emserr	dw	0

vrthand dw	0
vrtpags dw	0

codeseg
; inits memory for our use
proc setUpAuxMem uses si di, vmofs:word, vmseg:word
	; verify existance of xms memory
	mov ax, 4300h
	int 2fh
	cmp al, 80h
	jne no_xms

	mov ax, 4310h
	int 2fh
	mov [word ptr xmsaddr], bx
	mov [word ptr xmsaddr+2], es
	mov [usexms], 1

no_xms: ; now, verify existance of ems memory
	mov ax, 3567h
	int 21h
	mov di, 10
	mov si, offset emmname
	mov cx, 8
	cld
	repz cmpsb
	jne no_ems

	mov ah, 40h	; test ems status
	int 67h
	or ah, ah
	jne no_ems

	mov ah, 46h
	int 67h
	or ah, ah
	jne no_ems
	cmp al, 40h
	jb no_ems	; need ems 4.0+

	mov ah, 41h
	int 67h
	or ah, ah
	jnz no_ems
	mov [emsframe], bx

no_ems: ; now, create virtual memory
	push ds
	mov dx, [vmofs]
	mov ax, [vmseg]
	mov ds, ax
	mov ah, 3ch
	xor cx, cx
	int 21h
	pop ds
	jc no_vrt

	mov [vrthand], ax

no_vrt: ; bad file name or something
	ret
endp

proc _addToXMS
	mov bl, 0
	cmp [usexms], 0
	je noAddXMS

	cmp [xmshand], 0
	jne reallocxms

	; allocate new handle
	mov ah, 9
	mov dx, SIZE_K
	call [xmsaddr]
	or ax, ax
	je noAddXMS
	mov [xmshand], dx
	mov [xmssize], SIZE_K
	mov ax, 1
	ret

reallocxms:
	mov ah, 0fh
	mov bx, [xmssize]
	add bx, SIZE_K
	mov dx, [xmshand]
	call [xmsaddr]
	or ax, ax
	je noAddXMS
	add [xmssize], SIZE_K
	mov ax, [xmssize]
	mov cl, 2
	shr ax, cl
	ret

noAddXMS:
	mov bh, 0
	mov [xmserr], bx
	xor ax, ax
	ret
endp

proc _addToEMS
	cmp [emsframe], 0
	je noAddEMS

	cmp [emshand], 0
	jne reallocems

	; allocate new handle
	mov ah, 43h
	mov bx, 1
	int 67h
	or ah, ah
	jne noAddEMS
	mov [emshand], dx
	mov [emssize], 1
	mov ax, 1
	ret

reallocems:
	mov bx, [emssize]	; Number of our pages now
	inc bx			; Number of our pages soon

	mov ax, bx		; Save copy

	shr bx, 1
	shr bx, 1		; Divide by four

	and ax, 11b		; And add one more if partial...
	jz noPartialPage

	inc bx

noPartialPage:
	mov ah, 51h
	mov dx, [emshand]
	int 67h
	or ah, ah
	jne noAddEMS
	inc [emssize]
	mov ax, [emssize]
	ret

noAddEMS:
	mov al, ah
	mov ah, 0
	mov [emserr], ax
	xor ax, ax
	ret
endp

proc _addToVirt
	cmp [vrthand], 0
	je noAddVrt

	mov ax, [vrtpags]
	mov dx, SIZE_B
	mul dx
	mov cx, dx
	mov dx, ax
	mov bx, [vrthand]
	mov ax, 4200h
	int 21h
	jc noAddVrt

	mov ah, 40h	; it doesn't really matter where we write from.
	mov cx, SIZE_B	; actually, we may not need to write at all; i
	xor dx, dx	; forgot what happens if you seek beyond the end
	int 21h 	; of a file, and don't feel like checking it out.

	jc noAddVrt

	inc [vrtpags]
	mov ax, [vrtpags]
	ret

noAddVrt:
	xor ax, ax
	ret
endp

proc disposeAuxMem, vmofs:word, vmseg:word
	cmp [xmshand], 0
	je dsp_ems

	mov ah, 0ah
	mov dx, [xmshand]
	call [xmsaddr]
	mov [xmshand], 0
	mov [xmssize], 0
	mov [usexms], 0

dsp_ems:cmp [emshand], 0
	je dsp_vrt

	mov ah, 45h
	mov dx, [emshand]
	int 67h
	mov [emshand], 0
	mov [emssize], 0
	mov [emsframe], 0

dsp_vrt:mov bx, [vrthand]
	or bx, bx
	jz endDAM
	mov ah, 3eh
	int 21h 		; close the file

	push ds
	mov dx, [vmofs]
	mov ax, [vmseg]
	mov ds, ax
	mov ah, 41h
	int 21h 		; and delete it
	pop ds
	mov [vrthand], 0

endDAM: ret
endp

proc XMStoHeap uses si, tofs:word, tseg:word, fblk:word
	mov [dsthand], 0
	mov ax, [xmshand]
	mov [srchand], ax
	mov ax, [tofs]
	mov [word ptr dstofs], ax
	mov ax, [tseg]
	mov [word ptr dstofs+2], ax
	mov ax, [fblk]
	dec ax
	mov bx, SIZE_B
	mul bx
	mov [word ptr srcofs], ax
	mov [word ptr srcofs+2], dx
	mov si, offset xmsxfr
	mov ah, 0bh
	call [xmsaddr]

	mov bh, 0
	mov [xmserr], bx
	ret	; xms result is good for us, too
endp

proc EMStoHeap uses si di, tofs:word, tseg:word, fblk:word
	mov dx, [emshand]
	mov ax, 4400h
	mov bx, [fblk]
	dec bx
	mov cx, bx
	shr bx, 1
	shr bx, 1
	int 67h
	or ah, ah
	jne no_eth

	; Offset into EMS page...
	mov ax, cx
	and ax, 11b
	mov cx, SIZE_B
	mul cx
	mov si, ax

	push ds
	mov ax, [emsframe]
	mov bx, [tseg]
	mov di, [tofs]
	mov ds, ax
	mov es, bx

	mov cx, SIZE_B / 2
	cld
	rep movsw
	pop ds
	mov ax, 1
	ret

no_eth:	mov al, ah
	mov ah, 0
	mov [emserr], ax
	xor ax, ax
	ret
endp

proc VirttoHeap, tofs:word, tseg:word, fblk:word
	mov bx, [vrthand]
	or bx, bx
	je no_vth

	mov ax, [fblk]
	dec ax
	mov dx, SIZE_B
	mul dx
	mov cx, dx
	mov dx, ax
	mov ax, 4200h
	int 21h 	; seek the block
	jc no_vth

	push ds
	mov dx, [tofs]
	mov ax, [tseg]
	mov ds, ax
	mov cx, SIZE_B
	mov ah, 3fh
	int 21h 	; simple file read
	pop ds
	jc no_vth

	cmp ax, SIZE_B
	jne no_vth

	mov ax, 1
	ret

no_vth: xor ax, ax
	ret
endp

proc HeapToEMS uses si di, tblk:word, fofs:word, fseg:word
	mov dx, [emshand]
	mov ax, 4400h
	mov bx, [tblk]
	dec bx
	mov cx, bx
	shr bx, 1
	shr bx, 1
	int 67h
	or ah, ah
	jne no_hte

	; Offset into EMS page...
	mov ax, cx
	and ax, 11b
	mov cx, SIZE_B
	mul cx
	mov di, ax

	push ds
	mov ax, [emsframe]
	mov bx, [fseg]
	mov si, [fofs]
	mov ds, bx
	mov es, ax

	mov cx, SIZE_B / 2
	cld
	rep movsw
	pop ds
	xor ax, ax
	ret

no_hte:	mov al, ah
	mov ah, 0
	mov [emserr], ax
	ret
endp

proc HeapToXMS uses si, tblk:word, fofs:word, fseg:word
	mov [srchand], 0
	mov ax, [xmshand]
	mov [dsthand], ax
	mov ax, [fofs]
	mov [word ptr srcofs], ax
	mov ax, [fseg]
	mov [word ptr srcofs+2], ax
	mov ax, [tblk]
	dec ax
	mov bx, SIZE_B
	mul bx
	mov [word ptr dstofs], ax
	mov [word ptr dstofs+2], dx
	mov si, offset xmsxfr
	mov ah, 0bh
	call [xmsaddr]

	mov bh, 0
	mov [xmserr], bx
	ret	; xms result is good for us, too
endp

proc HeapToVirt, tblk:word, fofs:word, fseg:word
	mov bx, [vrthand]
	or bx, bx
	je no_htv

	mov ax, [tblk]
	dec ax
	mov dx, SIZE_B
	mul dx
	mov cx, dx
	mov dx, ax
	mov ax, 4200h
	int 21h
	jc no_htv	; seek the block

	push ds
	mov dx, [fofs]
	mov ax, [fseg]
	mov ds, ax
	mov cx, SIZE_B
	mov ah, 40h	
	int 21h 	; simple file write
	pop ds
	jc no_htv

	cmp ax, SIZE_B
	jne no_htv

	mov ax, 1
	ret

no_htv: xor ax, ax
	ret
endp

endif
end
