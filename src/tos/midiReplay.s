
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi delta handling and timing demo

    include "timing/mfp_m68k.inc"
    include "common_m68k.inc"
    
    xdef _midiSeqReplay	     ; custom sequence replay handler
    xref _updateStep	     ; our sequence update routine
    xref update		     ; our installed interrupt routine

	TEXT
_midiSeqReplay:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240	;change 1st color in palette (TODO: remove it in the final version)
	
	jsr 	_updateStep		;updates

	if (IKBD_MIDI_SEND_DIRECT==1)
	echo	"[VASM]***************** IKBD MIDI DATA SEND DIRECT ENABLED"
	move.l #_MIDIsendBuffer,a0
	move.w	_MIDIbytesToSend,d1
.loop:      
      cmpi.l	#0,d1	
      beq.s	.done
      moveq	#0,d0
	
      ;slap data to d0
      move.b	(a0)+,d0
.wait:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait		;no, wait!
      move.b	d0,$fffffc06.w	;write to MIDI data register

      subq.l	#1,d1
      bra.s	.loop
.done:
	move.w	#0,_MIDIbytesToSend 
	else
	echo	"[VASM]***************** IKBD MIDI DATA SEND DIRECT DISABLED"
	endif
	; prepare next tick
        move.l    update,$120		;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
.finish:	
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr.b	  #0,$fffffa0f  	; finished!
	rte                 		; return from timer
