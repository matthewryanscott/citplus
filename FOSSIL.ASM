; fossil com driver for citadel

ideal
model large, c

codeseg

org 0
desc	db 'Citadel Serial FOSSIL 1.00'
	db 32 - ((offset $ - offset desc)) dup(0)
				; filler for alignment

	dw offset init_port	; open port with params
	dw offset close_port	; close port

	dw offset ring_detect	; is the RI line active?
	dw offset have_carrier	; is the CD line active?

	dw offset mi_ready	; do we have stuff waiting?
	dw offset ib_flush	; flush the input buffer

	dw offset get_input	; get the next thing waiting
	dw offset put_output	; send this character...

	dw offset set_dtr	; raise or drop it
	dw offset ob_flush	; flush output buffer

	dw 118 dup (0)		; 128 total entries reserved


pinit	db 0
mport	dw 0

; -----------------------------------------------------------------------------
;       Port    0 = COMM1,      1 = COMM2,      2 = COMM3,      3 = COMM4
;
;	Baud	0 = 110,	1 = 150,	2 = 300,	3 = 600,
;		4 = 1200,	5 = 2400,	6 = 4800,	7 = 9600,
;		8 = 19200,	9 = 38400,	A = 57600,	B = 115200
;
;	Stops	0 = 1 stop bit, 1 = 2 stop bits
;
;	Parity	0 = none,	1 = odd,	2 = none,	3 = even
;
;	Len	0 = 5 bits,	1 = 6 bits,	2 = 7 bits,	3 = 8 bits
;
;	cCTS	0 = no check,	1 = check CTS on output
;
proc init_port port:word, baud:word, stops:word, parity:word, len:word, cCTS:word

; initialize port
	mov dx, [port]
	mov [cs:mport], dx
	mov ah, 4
	xor bx, bx
	int 14h
	cmp ax, 1954h
	jne no_init

	mov [cs:pinit], 1

; set baud and stuff (always set 8,n,1 - FOSSIL may not support other than
;			that, and Citadel never asks for other than that)
	cmp [baud], 2
	jbe is_300

	cmp [baud], 3
	je is_600

	cmp [baud], 4
	je is_1200

	cmp [baud], 5
	je is_2400

	cmp [baud], 6
	je is_4800

	cmp [baud], 7
	je is_9600

	cmp [baud], 8
	je is_19200

; 38400 is as high as fossil supports...
	mov al, 00100011b
	jmp short set_baud

is_300:
	mov al, 01000011b
	jmp short set_baud

is_600:
	mov al, 01100011b
	jmp short set_baud

is_1200:
	mov al, 10000011b
	jmp short set_baud

is_2400:
	mov al, 10100011b
	jmp short set_baud

is_4800:
	mov al, 11000011b
	jmp short set_baud

is_9600:
	mov al, 11100011b
	jmp short set_baud

is_19200:
	mov al, 00000011b


set_baud:;and stuff
	mov ah, 0
	mov dx, [port]
	int 14h

; set CTS
	mov ax, [cCTS]
	shl ax, 1
	mov ah, 0fh
	int 14h

no_init:ret
endp

; -----------------------------------------------------------------------------
proc close_port
	cmp [cs:pinit], 1
	jne no_cp

	mov ah, 5
	mov dx, [cs:mport]
	int 14h
	mov [cs:pinit], 1
no_cp:	ret
endp

; -----------------------------------------------------------------------------
; FOSSIL does not provide for RI tracking
proc ring_detect
	xor ax, ax
	ret
endp

; -----------------------------------------------------------------------------
proc have_carrier
	cmp [cs:pinit], 1
	jne no_hc

	mov ah, 3
	mov dx, [cs:mport]
	int 14h

	test al, 80h
	jz no_hc

	; have carrier...
	mov ax, 1
	ret

no_hc:	xor ax, ax
	ret
endp

; -----------------------------------------------------------------------------
proc mi_ready
	cmp [cs:pinit], 1
	jne no_hc

	mov ah, 3
	mov dx, [cs:mport]
	int 14h

	test ah, 1
	jz no_hc

	mov ax, 1
        ret
endp

; -----------------------------------------------------------------------------
proc ib_flush
	cmp [cs:pinit], 1
	jne no_if

	mov ah, 0ah
	mov dx, [cs:mport]
	int 14h

no_if:	ret
endp

; -----------------------------------------------------------------------------
proc get_input
	cmp [cs:pinit], 1
	jne no_gi

	mov ah, 2
	mov dx, [cs:mport]
	int 14h

no_gi:	ret
endp

; -----------------------------------------------------------------------------
proc put_output char:byte
	cmp [cs:pinit], 1
	jne no_po

	mov ah, 1
	mov al, [char]
	mov dx, [cs:mport]
	int 14h

no_po:	ret
endp

; -----------------------------------------------------------------------------
proc set_dtr setit:byte
	cmp [cs:pinit], 1
	jne no_sd

	mov al, [setit]
	mov ah, 6
	mov dx, [cs:mport]
	int 14h

no_sd:	ret
endp

; -----------------------------------------------------------------------------
proc ob_flush
	cmp [cs:pinit], 1
	jne no_of

	mov ah, 8
	mov dx, [cs:mport]
	int 14h

no_of:	ret
endp


end
