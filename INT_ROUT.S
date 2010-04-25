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
	XDEF	_myRoutine 
	XDEF	_installTickCounter
	XDEF	_deinstallTickCounter
	XDEF    _counter
; export labels 
	text
_myRoutine:
	move.l	#text,-(sp)
	move.w	#$09,-(sp)
	trap	#1
	addq.l	#6,sp
	rts

; installs tick counter on timer B
_installTickCounter:
	bsr.w	  super_on
	clr.b     $fffffa1b
	move.l	  #0,_counter	;init counter 
	move.l	  $120,_oldTB	

	move.l    #_tickCounter,$120	;slap interrupt 
	bset      #0,$fffffa13
	bset      #0,$fffffa07
	
	move.b    #127,$fffffa21	;data 59 for ~4166
	move.b    #7,$fffffa1b		;div 10
	bsr.w	  super_off
	rts

_deinstallTickCounter:
	bsr.w	super_on
	clr.b     $fffffa1b
	move.l	 _oldTB,$120
	bsr.w	super_off
	rts

_tickCounter:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	clr.b     $fffffa1b
	eor.w	#$0f0,$ffff8240
	move.l	  _counter,d0

	add.l	#1,d0
	move.l	d0,_counter

	move.l    #_tickCounter,$120	;slap interrupt 
	bset      #0,$fffffa13
	bset      #0,$fffffa07
	
	move.b    #127,$fffffa21	;data 59 for ~4166
	move.b    #7,$fffffa1b		;div 10

	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr  #0,$fffffa0f  ; finished!
	rte                 ; return from timer

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
	move.l	d0,old_ssp
	movem.l	(sp)+,d0/a0 
	RTS


	bss
_counter: 	ds.l	1
_oldTB:	  	ds.l	1
old_ssp:	ds.l	1

	data
text:	 dc.b 	'Test test...............',$0d,$0a,$00


	


	
