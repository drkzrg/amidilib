
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi delta handling and timing demo

    include "timing/mfp_m68k.inc"

    xdef _midiOutputEnabled     ;midi out enabled flag
    xdef _ymOutputEnabled	;ym2149 output enable flag
    xdef _customSeqReplay	;custom sequence replay handler
    
    xref _tbMode	;data feed to timer, which control
    xref _tbData	;individual tick duration
    
    xref _updateSequenceStep	;our sequence update routine
    xref update			;our installed interrupt routine
	even
_customSeqReplay:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240	;change 1st color in palette (TODO: remove it in the final version)
	
	jsr 	_updateSequenceStep

	;prepare next tick
        move.l    update,$120		;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
.finish:	
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr.b	  #0,$fffffa0f  	; finished!
	rte                 		; return from timer

	bss

	even
_midiOutputEnabled:	ds.l	1
_ymOutputEnabled: 	ds.l	1
	align 4

