;================================================
;    Copyright 2007-2011 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.
;================================================

AMIDI_MAX_TRACKS equ	16

       xdef _installReplayRout		;initialises replay interrupt TB routine and prepares data
       xdef _deinstallReplayRout	;removes replay routine from system 
	   
       xdef _oldTB			;saved old Timer B vector
       xdef _tbData
       xdef _tbMode

       xref _sequenceUpdate
       xref _pCurrentSequence
       xref super_off
       xref super_on

_installReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	  super_on
	move.w	sr,-(sp)	;save status register
	or.w	#$0700,sr	;turn off all interupts

	move.l	$42(sp),d1  ;mode
        move.l  $46(sp),d0  ;data
        
	move.b	d1,_tbMode  		;save parameters for later
	move.b	d0,_tbData

	clr.b     $fffffa1b		;turn off tb
	
	move.l	  $120,_oldTB	
	move.l    #update,$120		;slap interrupt
	
	move.b    d0,$fffffa21		;put data 
	move.b    d1,$fffffa1b		;put mode 
	bset.b    #0,$fffffa07
	bset.b    #0,$fffffa13
	
	move.w 	  (sp)+,sr 		;restore Status Register
	bsr.w	  super_off
	movem.l (sp)+,d0-d7/a0-a6		;restore registers
	rts

update:
      movem.l   d0-7/a0-6,-(a7)	;save registers
      ;move.w	sr,-(sp)	;save status register
      ;or.w	#$0700,sr	;turn off all interupts 

      clr.b     $fffffa1b
      eor.w	#$0f0,$ffff8240	;change 1st color in palette (TODO: remove it in the final version)

      ;check pulses per quaternote
      move.l	_pCurrentSequence,a0
      move.l	pulseCounter(a0),d1
      cmpi.l	#0,d1
      bne.s	.nextTick		;if pulseCounter==timedivision-1
      jsr	_sequenceUpdate		;jump to sequence handler, sneaky bastard ;>
      
      move.l	_pCurrentSequence,a0
      move.l	divider(a0),pulseCounter(a0)
   
      bra.s	.setInt			;set up timers and finish
.nextTick:				;we didn't reach the proper pulse amount
	subq.l	#1,d1			;decrease counter
	move.l	d1,pulseCounter(a0)	;save it
.setInt:
      ;prepare next tick
      move.l    #update,$120		;slap interrupt 
      move.b    _tbData,$fffffa21	;set data
      move.b    _tbMode,$fffffa1b	;div mode
      bset.b    #0,$fffffa07		;go!
      bset.b    #0,$fffffa13

      ;move.w 	  (sp)+,sr 		;restore Status Register
      movem.l   (a7)+,d0-7/a0-6		;restore registers
      bclr.b	  #0,$fffffa0f  	; finished!
      rte

; deinstalls MIDI replay on timer B 
_deinstallReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	super_on

	move.w	sr,-(a7)		;save status register
	or.w	#$0700,sr

	clr.b     $fffffa1b	;turn off tb
	move.l	 _oldTB,$120	;save old tb
	
        move.w	(sp)+,sr	;restore Status Register

	bsr.w	super_off
	movem.l (sp)+,d0-d7/a0-a6
	rts

	bss
	even
_oldTB:			ds.l	1
	even
_tbData:		ds.b	1
	even
_tbMode:		ds.b	1

;sSequence_t structure
   RSRESET
pSequenceName	rs.l	1	; NULL terminated string ptr
arTracks	rs.l	AMIDI_MAX_TRACKS; up to AMIDI_MAX_TRACKS (16) tracks available
timeDivision	rs.l	1	; pulses per quater note(time division) 
eotThreshold	rs.l	1	; see define EOT_SILENCE_THRESHOLD 

accumDelta	rs.l	1	; accumulated delta counter 
pulseCounter	rs.l	1	; pulses per quaternote counter 
divider		rs.l	1	;

ubNumTracks	rs.b	1	; number of tracks 
ubDummy		rs.b	3	;
ubActiveTrack	rs.b	1	; range 0-(ubNumTracks-1) tracks 
ubDummy1	rs.b	3	;

  RSRESET