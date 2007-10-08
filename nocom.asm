; fossil com driver for citadel

ideal
model large, c

codeseg

org 0
desc    db 'Citadel No-COM-port Driver'
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

    xor ax, ax
    ret
endp

; -----------------------------------------------------------------------------
proc close_port
     xor ax, ax
     ret
endp

; -----------------------------------------------------------------------------
; FOSSIL does not provide for RI tracking
proc ring_detect
	xor ax, ax
	ret
endp

; -----------------------------------------------------------------------------
proc have_carrier
    xor ax, ax
	ret
endp

; -----------------------------------------------------------------------------
proc mi_ready
    xor ax, ax
        ret
endp

; -----------------------------------------------------------------------------
proc ib_flush
    xor ax, ax
    ret
endp

; -----------------------------------------------------------------------------
proc get_input
    xor ax, ax
    ret
endp

; -----------------------------------------------------------------------------
proc put_output char:byte
    xor ax, ax
    ret
endp

; -----------------------------------------------------------------------------
proc set_dtr setit:byte
    xor ax, ax
    ret
endp

; -----------------------------------------------------------------------------
proc ob_flush
    xor ax, ax
    ret
endp


end

