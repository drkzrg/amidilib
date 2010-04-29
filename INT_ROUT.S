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
	
	
          xdef _installTickCounter
	  xdef deinstallTickCounter
	  xdef	_installTA
	  xdef	_counter
	  xdef  _cA

; installs tick counter on timer B
_installTickCounter:	 
	movem.l	d0-a6,-(sp)
	bsr.w	  super_on
	
	move.w	sr,-(sp)	;save status register

	or.w	#$0700,sr

	clr.b     $fffffa1b
	move.l	  #0,_counter		;init counter 
	move.l	  $120,_oldTB	
	move.l    #tickCounter,$120	;slap interrupt
	
	move.b    #127,$fffffa21	;data 59 for ~4166
	move.b    #7,$fffffa1b		;div 10 
	bset.b    #0,$fffffa07
	bset.b    #0,$fffffa13
	move.w (sp)+,sr 		;restore Status Register
	bsr.w	  super_off
	movem.l (sp)+,d0-a6 		;restore registers
	
	rts

_installTA:
	bsr.w	super_on
	pea	_tiA
	move.w	#245,-(sp)
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

_deinstallTickCounter:
	bsr.w	super_on
	move.w	sr,-(a7)		;save status register

	or.w	#$0700,sr

	clr.b     $fffffa1b	;turn off tb
	move.l	 _oldTB,$120	;install old tb
	
        move.w	(sp)+,sr		;restore Status Register
	movem.l	(sp)+,d0-a6		;restore registers
	bsr.w	super_off
	rts




tickCounter:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240
	lea.l	_counter,a0
   	move.l	(a0),d0
	addq.l	#1,d0
	move.l	d0,(a0)
	

	move.l    #tickCounter,$120	;slap interrupt 
	move.b    #127,$fffffa21	;data 59 for ~4166
	move.b    #7,$fffffa1b		;div 10 
	bset.b    #0,$fffffa07
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

super_on:
	movem.l	d0/a0,-(sp)
	clr.l	-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	move.l	d0,old_ssp
	movem.l	(sp)+,d0/a0
	RTS

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
	data
text:	dc.b 	'Test test...............',$0d,$0a,$00
	even

	


	
