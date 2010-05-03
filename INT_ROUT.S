; Copyright 2008, 2009, 2010 Pawel Goralski
; e-mail: pawel.goralski@nokturnal.pl
; This file is part of AMIDILIB.

; AMIDILIB is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.

; AMIDILIB is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.
; export routines 
	
	
          xdef _installMIDIreplay
	  xdef _deinstallMIDIreplay
	  xdef	_installTA
	  xdef	_counter
	  xdef  _cA

;external references
	  xref  am_send_midi_cmd

;MFP MK68901 Multi Function Periferal
MFP_STOP	equ 	%0000  ;Timer stop
MFP_DIV4	equ 	%0001  ;div 4
MFP_DIV10	equ	%0010  ;div 10
MFP_DIV16	equ     %0011  ;div 16
MFP_DIV50	equ	%0100  ;div 50
MFP_DIV64	equ	%0101  ;div 64
MFP_DIV100	equ	%0110  ;div 100
MFP_DIV200	equ	%0111  ;div 200
MFP_EC		equ	%1000  ;event count mode (TA/TB) 
MFP_DEL4	equ 	%1001  ;delay 4
MFP_DEL10	equ	%1010  ;delay 10
MFP_DEL16	equ     %1011  ;delay 16
MFP_DEL50	equ	%1100  ;delay 50
MFP_DEL64	equ	%1101  ;delay 64
MFP_DEL100	equ	%1110  ;delay 100
MFP_DEL200	equ	%1111  ;delay 200;

; installs MIDI replay on timer B
_installMIDIreplay:	 
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	  super_on
	move.w	sr,-(sp)	;save status register
        or.w	#$0700,sr	;turn off all interupts

	move.w	$54(sp),d1  ;mode
	move.w  $58(sp),d0  ;data
        move.b	d1,_tbMode  ;save parameters for later
	move.b	d0,_tbData

	clr.b     $fffffa1b		;turn off tb
	move.l	  #0,_counter		;init counter 
	move.l	  $120,_oldTB	
	move.l    #tickCounter,$120	;slap interrupt
	
	move.b    d0,$fffffa21		;put data 
	move.b    d1,$fffffa1b		;put mode 
	bset.b    #0,$fffffa07
	bset.b    #0,$fffffa13
	
	move.w 	  (sp)+,sr 		;restore Status Register
	bsr.w	  super_off
	movem.l (sp)+,d0-d7/a0-a6		;restore registers
	rts

; deinstalls MIDI replay on timer B 
_deinstallMIDIreplay:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	super_on

	move.w	sr,-(a7)		;save status register
	or.w	#$0700,sr

	clr.b     $fffffa1b	;turn off tb
	move.l	 _oldTB,$120	;save old tb
	
        move.w	(sp)+,sr		;restore Status Register

	bsr.w	super_off
	movem.l (sp)+,d0-d7/a0-a6
	rts

_installTA:
	bsr.w	super_on
	pea	_tiA
	move.w	#59,-(sp)
	move.w	#6,-(sp)
	move.w	#0,-(sp)
	move.w	#$1f,-(sp)
	trap	#14
;enable interrupt
	move.w	#13,-(sp)
	move.w	#$1b,-(sp)
	trap	#14
	addq.l	#4,sp
	lea	$c(sp),sp
	bsr.w	super_off
	rts

tickCounter:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240
	
	lea.l	_counter,a0		;increase counter
   	move.l	(a0),d0
	addq.l	#1,d0
	move.l	d0,(a0)

	move.l    #tickCounter,$120	;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
	
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr.b	  #0,$fffffa0f  ; finished!
	rte                 ; return from timer

_tiA:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	
	lea.l	_cA,a0
   	move.l	(a0),d0
	addq.l	#1,d0
	move.l	d0,(a0)

	eor.w	  #$00f,$ffff8240
	bclr.b	  #5,$fffffa0f  ; finished!
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	rte
;enter supervisor mode
super_on:
	movem.l	d0/a0,-(sp)
	clr.l	-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	move.l	d0,old_ssp
	movem.l	(sp)+,d0/a0
	RTS

;leave supervisor mode
super_off:
	movem.l	d0/a0,-(sp)
	move.l	old_ssp,-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	movem.l	(sp)+,d0/a0 
	RTS


	bss
	even
_oldTB:	  	ds.l	1
old_ssp:	ds.l	1
_counter:	ds.l	1
_cA: 		ds.l 	1
	even
_tbMode:	ds.b	1
	even
_tbData:	ds.b	1


	


	
