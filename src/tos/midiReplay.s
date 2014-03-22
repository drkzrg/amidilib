
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi sequence replay from custom structure

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
	echo	"[midiReplay.s] IKBD MIDI DATA SEND DIRECT ENABLED"

	move.w	#0,d1
	move.l 	#_MIDIsendBuffer,a0
	move.w	_MIDIbytesToSend,d1

        cmpi.w   #0,d1
        beq.s   .done       ;if 0 bytes do nothing
.send:      
      ;slap data to d0
      move.w	(a0),d0	;get word
      clr.w	(a0)+ 	;clear it
      
      move.w	d0,d2		;make copy
      andi.w	#$FF00,d2
      lsr.w	#8,d2
.wait1:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait1		;no, wait!
      move.b	d2,$fffffc06.w	;write to MIDI data register
      subq.w	#1,d1
      cmpi.w	#0,d1	
      beq.s	.done
      
      ;not done
      move.w	d0,d2
      andi.w	#$00FF,d2
.wait2:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait2		;no, wait!
      move.b	d2,$fffffc06.w	;write to MIDI data register
      
      subq.w	#1,d1
      cmpi.w	#0,d1	
      beq.s	.done
      
      bra.s	.send
.done:
	move.w	#0,_MIDIbytesToSend 
	else
	echo	"[midiReplay.s] IKBD MIDI DATA SEND DIRECT DISABLED"
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

