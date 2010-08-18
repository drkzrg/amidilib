;   Copyright 2007-2010 Pawel Goralski
;   e-mail: pawel.goralski@nokturnal.pl
;   This file is part of AMIDILIB.
;   See license.txt for licensing information.

	   xdef super_on		; self explanatory 
	   xdef super_off		;
	   xdef _turnOffKeyclick	;

_turnOffKeyclick:
      bsr.w	super_on
      bclr	#0,$484.w

      bsr.w	super_off
      rts


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
old_ssp:		ds.l	1








