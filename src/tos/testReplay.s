
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi delta handling and timing demo

    include "timing/mfp_m68k.inc"

    xdef _installYMReplayRout	;initialises replay interrupt TB routine and prepares data
    xdef _deinstallYMReplayRout	;removes replay routine from system 
    xdef _midiOutputEnabled     ;midi out enabled flag
    xdef _ymOutputEnabled	;ym2149 output enable flag
    
    xref _tbMode	;data feed to timer, which control
    xref _tbData	;individual tick duration
    xref _oldTB		;saved old Timer B vector
    
    xref _ymSoundOff
    xref _am_allNotesOff
    
    xref _updateSequenceStep	;our sequence update routine
    xref _super_on	;self explanatory 
    xref _super_off	;

_installYMReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	  _super_on
	move.w	sr,-(sp)	;save status register
        or.w	#$0700,sr	;turn off all interupts

	move.l	$42(sp),d1  ;mode
        move.l  $46(sp),d0  ;data
        
	move.b	d1,_tbMode  ;save parameters for later
	move.b	d0,_tbData

	clr.b     $fffffa1b		;turn off tb
	
	move.l	  $120,_oldTB	
	move.l    #update,$120		;slap interrupt
	
	move.b    d0,$fffffa21		;put data 
	move.b    d1,$fffffa1b		;put mode 
	bset.b    #0,$fffffa07
	bset.b    #0,$fffffa13
	
	move.w 	  (sp)+,sr 		;restore Status Register
	bsr.w	  _super_off();
	movem.l (sp)+,d0-d7/a0-a6	;restore registers
	rts

update:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240	;change 1st color in palette (TODO: remove it in the final version)
	
	jsr 	_updateSequenceStep

	;prepare next tick
        move.l    #update,$120		;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
.finish:	
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr.b	  #0,$fffffa0f  	; finished!
	rte                 		; return from timer

; deinstalls MIDI replay on timer B 
_deinstallYMReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	_super_on

	move.w	sr,-(a7)		;save status register
	or.w	#$0700,sr

	clr.b     $fffffa1b	;turn off tb
	move.l	 _oldTB,$120	;save old tb
	
        move.w	(sp)+,sr	;restore Status Register

	bsr.w	_super_off();
	movem.l (sp)+,d0-d7/a0-a6
	rts

	bss
	even
_midiOutputEnabled:	ds.l	1
_ymOutputEnabled: 	ds.l	1
_oldTB:			ds.l	1

