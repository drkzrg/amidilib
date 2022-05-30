;	IKBD 6301 interrupt routine
;    
;	Copyright (C) 2002	Patrice Mandin
;	Copyright (C) 2011-22 Pawel Goralski (devpac/vasm version)
	
IKBD_TABLE_SIZE	equ	128

KEY_PRESSED		equ	$ff
KEY_UNDEFINED	equ	$80
KEY_RELEASED	equ	$00

	TEXT

	XDEF	_IkbdInstall
	XDEF	_IkbdUninstall
	XDEF    _IkbdClearState

	XDEF	_Ikbd_keyboard
	XDEF	_Ikbd_mouseb
	XDEF	_Ikbd_mousex
	XDEF	_Ikbd_mousey
	XDEF	_Ikbd_joystick

; --- Install our IKBD vector ---

_IkbdInstall:
	movem.l	d0-d1/a0-a1,-(sp)

	; Disable all interrupts
	move.w  sr,d1
	move.w	#$2700,sr

	; Save MFP registers used for ACIA interrupt
	lea		$fffffa00.w,a0
	btst	#6, $09(a0)
	sne		ikbd_ierb
	btst	#6,$15(a0)
	sne		ikbd_imrb

	; Set our MFP routine

	move.l	$118.w,old_ikbd
	move.l	#ikbd,$118.w
	bset	#6,$fffffa09.w	; IERB
	bset	#6,$fffffa15.w	; IMRB

	; Set relative mouse motion mode
	; needed because running a .tos or .ttp program
	; disable the mouse (entering VT52 textmode)

	move.b	#$08,$fffffc02.w

	; Re-enable interrupts
	move.w	d1,sr

	movem.l	(sp)+,d0-d1/a0-a1
	rts

;--- Uninstall our IKBD vector ---

_IkbdUninstall:
	move.l	a0,-(sp)

	; Disable interrupts
	move.w  sr,d1
	move.w	#$2700,sr

	; Restore previous MFP registers

	lea	$fffffa00.w,a0

	bclr	#6,$09(a0)
	tst.b	ikbd_ierb
	beq.s	.ikbd_restoreierb
	bset	#6,$09(a0)

.ikbd_restoreierb:
	bclr	#6,$15(a0)
	tst.b	ikbd_imrb
	beq.s	.ikbd_restoreimrb
	bset	#6,$15(a0)

.ikbd_restoreimrb:
	move.l	old_ikbd,$118.w

	; Clear keyboard buffer

	lea	$fffffc00.w,a0
.ikbd_videbuffer:
	btst	#0,(a0)
	beq.s	.ikbd_finbuffer
	tst.b	$02(a0)
	bra.s	.ikbd_videbuffer

.ikbd_finbuffer:
	; Re-enable interrupts
	move.w	d1,sr
	move.l	(sp)+,a0
	rts

	BSS

	EVEN
ikbd_ierb:	
	ds.b	1
	EVEN
ikbd_imrb:
	ds.b	1

; --- Our custom IKBD vector ---

	TEXT
	EVEN
	dc.b	"XBRA"
	dc.b	"LAMD"
old_ikbd: 
	ds.b 4
ikbd:
	; test if byte coming from IKBD or MIDI
	movem.l	d0-d1/a0,-(sp)

	btst    #0,$fffffc00.w
	;beq.s	ikbd_oldmidi        ; jump to old midi handler
	beq.s	ikbd_endit 			; do nothig

	move.b	$fffffc02.w,d0

	; Joystick packet ?
	
	cmp.b	#$ff,d0
	beq.s	.ikbd_yes_joystick

	; Mouse packet ?

	cmp.b	#$f8,d0
	bmi.s	.ikbd_no_mouse
	cmp.b	#$fc,d0
	bpl.s	.ikbd_no_mouse

.ikbd_yes_mouse:
	and.w	#3,d0
	move.w	d0,_Ikbd_mouseb

	move.l	#ikbd_mousex,$118.w
	bra.s	.ikbd_endit_stack

.ikbd_yes_joystick:
	move.l	#ikbd_joystick,$118.w
	bra.s	.ikbd_endit_stack

	; Keyboard press/release

.ikbd_no_mouse:
	move.b	d0,d1
	lea	_Ikbd_keyboard,a0
	and.l	#$7f,d1
	tas	d0
	spl	0(a0,d1.w)

.ikbd_endit_stack:
	movem.l	(sp)+,d0-d1/a0
ikbd_endit:
	bclr	#6,$fffffa11.w
	rte

	; call old MIDI interrupt
ikbd_oldmidi:
	movem.l	(sp)+,d0-d1/a0
	move.l	old_ikbd,-(sp)
	rts

ikbd_mousex:
	; test if byte coming from IKBD or MIDI

	btst	#0,$fffffc00.w
	beq.s	ikbd_endit

	move.w	d0,-(sp)

	move.b	$fffffc02.w,d0
	ext.w	d0
	add.w	d0,_Ikbd_mousex

	move.w	(sp)+,d0

	move.l	#ikbd_mousey,$118.w
	bra.s	ikbd_endit

ikbd_mousey:
	; test if byte coming from IKBD or MIDI

	btst	#0,$fffffc00.w
	beq.s	ikbd_endit

	move.w	d0,-(sp)

	move.b	$fffffc02.w,d0
	ext.w	d0
	add.w	d0,_Ikbd_mousey

	move.w	(sp)+,d0

	move.l	#ikbd,$118.w
	bra.s	ikbd_endit

ikbd_joystick:
	; test if byte coming from IKBD or MIDI

	btst	#0,$fffffc00.w
	beq.s	ikbd_endit

	move.b	$fffffc02.w,_Ikbd_joystick+1

	move.l	#ikbd,$118.w
	bra.s	ikbd_endit

_IkbdClearState:
	movem.l	d0/a0,-(sp)
	
	move.w  #$8080,d0					
	lea.l   _Ikbd_keyboard,a0

	rept    64
	move.w 	d1,(a0)+
	endr
	
	move.w  #$0000,d0
	lea.l   _Ikbd_mousex,a0

	move.w 	d0,(a0)+
	move.w 	d0,(a0)+
	move.w 	d0,(a0)+

	movem.l (sp)+,d0/a0
	rts

	BSS

	EVEN
_Ikbd_keyboard: ds.b IKBD_TABLE_SIZE
_Ikbd_mousex: 	ds.b 2
_Ikbd_mousey:	ds.b 2
_Ikbd_mouseb:	ds.b 2
_Ikbd_joystick:	ds.b 2
