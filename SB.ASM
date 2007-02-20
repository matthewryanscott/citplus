; sound blaster sound driver for citadel
; -- sbsim is used for interface to hardware

ideal
model large, c

codeseg

org 0
desc	db 'Citadel Soundblaster 1.00'
	db 32 - ((offset $ - offset desc)) dup(0)
				; filler for alignment

	dw offset init_sound	; init driver
	dw offset close_sound	; close driver

	dw offset get_version	; get sbsim version
	dw offset query_drivers ; which are loaded?
	dw offset query_status	; which are currently active?

	dw offset start_snd_src ; wow
	dw offset play_sound	; wow
	dw offset stop_sound	; wow
	dw offset pause_sound	; wow
	dw offset resume_sound	; wow
	dw offset read_snd_stat ; wow

	dw offset set_midi_map	; wow

	dw offset get_src_vol	; wow
	dw offset set_src_vol	; wow
	dw offset set_fade_pan	; wow
	dw offset strt_fade_pan ; wow
	dw offset stop_fade_pan ; wow
	dw offset pse_fade_pan	; wow
	dw offset res_fade_pan	; wow
	dw offset read_fade_pan ; wow
	dw offset get_pan_pos	; wow
	dw offset set_pan_pos	; wow

	dw offset say_ascii	; wow

	dw 105 dup (0)		; 128 total entries reserved

talk	db 0
talkadr dd 0
talkbuf dd 0

proc do_int near
	db 0cdh 		; int
dvr_int db 0
	ret
endp

; -----------------------------------------------------------------------------
proc init_sound
	cmp [dvr_int], 0	; already initialized?
	jne is_ret

	mov bx, 7fh * 4 	; offs of one less than first sbsim int
	xor ax, ax		; segment of ivt

wrong_int:
	add bx, 4		; next int offset
	cmp bx, 0c0h * 4
	jae no_int
	mov es, ax		; and segment

	mov dx, [es:bx+2]	; segment in dx
	mov es, dx		; then es...  [es:103h] is signature

	cmp [word ptr es:103h], 'BS'
	jne wrong_int
	cmp [word ptr es:105h], 'IS'
	jne wrong_int
	cmp [byte ptr es:107h], 'M'
	jne wrong_int

	shr bx, 1
	shr bx, 1
	mov [dvr_int], bl
	jmp short sbtalker

no_int: mov [dvr_int], 0

; look for sbtalker
sbtalker:
	mov [talk], 0

	mov dx, [es:0bch]	; int 2f segment
	or dx, dx
	jz is_ret

	mov es, dx
	mov ax, 0fbfbh
	int 2fh
	or ax, ax
	jnz is_ret

	mov ax, [es:bx+4]
	mov dx, [es:bx+6]

	mov [word ptr talkadr], ax
	mov [word ptr talkadr+2], dx

	add bx, 20h
	mov [word ptr talkbuf], bx
	mov [word ptr talkbuf+2], es

	mov [talk], 1

is_ret: mov al, [dvr_int]
	mov ah, 0
	ret
endp


; -----------------------------------------------------------------------------
proc close_sound
	mov [dvr_int], 0
	mov [talk], 0
	xor ax, ax
	ret
endp

; -----------------------------------------------------------------------------
proc get_version
	xor ax, ax
	cmp [dvr_int], 0
	je gv_ret

	xor bx, bx
	call do_int

gv_ret: ret
endp

; -----------------------------------------------------------------------------
proc query_drivers
	xor ax, ax
	cmp [dvr_int], 0
	je qd_ret

	mov bx, 1
	call do_int

qd_ret: ret
endp

; -----------------------------------------------------------------------------
proc query_status
	xor ax, ax
	cmp [dvr_int], 0
	je qs_ret

	mov bx, 3
	call do_int

qs_ret: ret
endp

; -----------------------------------------------------------------------------
proc start_snd_src which:byte, what:dword
	mov ax, -1
	cmp [dvr_int], 0
	je sss_ret

	mov bh, [which]
	mov bl, 0
	mov ax, [word ptr what]
	mov dx, [word ptr what+2]
	push ds
	mov ds, dx
	call do_int
	pop ds

sss_ret:ret
endp

; -----------------------------------------------------------------------------
proc play_sound which:byte
	mov ax, -1
	cmp [dvr_int], 0
	je ps_ret

	mov bh, [which]
	mov bl, 1
	call do_int

ps_ret: ret
endp

; -----------------------------------------------------------------------------
proc stop_sound which:byte
	mov ax, -1
	cmp [dvr_int], 0
	je ss_ret

	mov bh, [which]
	mov bl, 2
	call do_int
	xor ax, ax

ss_ret: ret
endp

; -----------------------------------------------------------------------------
proc pause_sound which:byte
	mov ax, -1
	cmp [dvr_int], 0
	je pse_ret

	mov bh, [which]
	mov bl, 3
	call do_int
	xor ax, ax

pse_ret:ret
endp

; -----------------------------------------------------------------------------
proc resume_sound which:byte
	mov ax, -1
	cmp [dvr_int], 0
	je rs_ret

	mov bh, [which]
	mov bl, 4
	call do_int
	xor ax, ax

rs_ret: ret
endp

; -----------------------------------------------------------------------------
proc read_snd_stat which:byte
	mov ax, -1
	cmp [dvr_int], 0
	je rss_ret

	mov bh, [which]
	mov bl, 5
	call do_int

rss_ret:ret
endp

; -----------------------------------------------------------------------------
proc set_midi_map map:word
	mov ax, -1
	cmp [dvr_int], 0
	je smm_ret

	mov bx, 0506h
	mov ax, [map]
	call do_int
	xor ax, ax

smm_ret:ret
endp

; -----------------------------------------------------------------------------
proc get_src_vol which:word
	xor ax, ax
	cmp [dvr_int], 0
	je gsv_ret

	mov bx, 0400h
	mov ax, [which]
	call do_int

gsv_ret:ret
endp

; -----------------------------------------------------------------------------
proc set_src_vol which:word, vol:word
	mov ax, -1
	cmp [dvr_int], 0
	je ssv_ret

	mov bx, 0401h
	mov ax, [which]
	mov dx, [vol]
	call do_int

ssv_ret:ret
endp

; -----------------------------------------------------------------------------
proc set_fade_pan fps:dword
	mov ax, -1
	cmp [dvr_int], 0
	je sfp_ret

	mov bx, 0410h
	mov ax, [word ptr fps]
	mov dx, [word ptr fps+2]
	call do_int

sfp_ret:ret
endp

; -----------------------------------------------------------------------------
proc strt_fade_pan
	mov ax, -1
	cmp [dvr_int], 0
	je bfp_ret

	mov bx, 0411h
	call do_int

bfp_ret:ret
endp

; -----------------------------------------------------------------------------
proc stop_fade_pan which:word
	mov ax, -1
	cmp [dvr_int], 0
	je tfp_ret

	mov bx, 0412h
	mov ax, [which]
	call do_int

tfp_ret:ret
endp

; -----------------------------------------------------------------------------
proc pse_fade_pan
	mov ax, -1
	cmp [dvr_int], 0
	je pfp_ret

	mov bx, 0413h
	call do_int

pfp_ret:ret
endp

; -----------------------------------------------------------------------------
proc res_fade_pan
	mov ax, -1
	cmp [dvr_int], 0
	je rfp_ret

	mov bx, 0414h
	call do_int

rfp_ret:ret
endp

; -----------------------------------------------------------------------------
proc read_fade_pan which:word
	mov ax, -1
	cmp [dvr_int], 0
	je dfp_ret

	mov bx, 0415h
	mov ax, [which]
	call do_int

dfp_ret:ret
endp

; -----------------------------------------------------------------------------
proc get_pan_pos which:word
	mov ax, -1
	cmp [dvr_int], 0
	je gpp_ret

	mov bx, 0416h
	mov ax, [which]
	call do_int

gpp_ret:ret
endp

; -----------------------------------------------------------------------------
proc set_pan_pos which:word, pos:word
	mov ax, -1
	cmp [dvr_int], 0
	je spp_ret

	mov bx, 0417h
	mov ax, [which]
	mov dx, [pos]
	call do_int

spp_ret:ret
endp

proc say_ascii uses si di, what:dword, len:byte
	mov ax, -1
	cmp [talk], 0
	je sa_ret

	mov es, [word ptr talkbuf+2]
	mov di, [word ptr talkbuf]

	mov al, [len]
	stosb

	push ds
	mov ds, [word ptr what+2]
	mov si, [word ptr what]
	mov cl, al
	mov ch, 0
	rep movsb
	pop ds

	mov ax, 707h
	call [talkadr]
	xor ax, ax

sa_ret:
	ret
endp


end
