;================================================
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.
;================================================

;TODO: remove it/obsolete/not used..
    include "common_m68k.inc"

;export symbols
    xdef _pCurrentSequence	;current sequence pointer
    
    xdef _startPlaying		;to let us now if there is an end of sequence
				;so MIDI beat 0 is send only once
    xdef _bTempoChanged		;flags indicating tempo and time signature change
    xdef _bTimeSignatureChanged
    xdef _updateMidiFunc	;our replay routine installed on interrupt
    
    ;import symbols
    xref _sequenceUpdate

	;not used, remove it
_updateMidiFunc:
      movem.l   d0-7/a0-6,-(a7)	;save registers
      clr.b     $fffffa1b

      if (MIDI_CLOCK_ENABLE==1)
      ;check if we are starting replay 
      move.w	_startPlaying,d0
      cmpi.w	#1,d0
      bne.s	.skipMidiStart
      move.w	#0,_startPlaying	

      move.b	#MIDI_START,d0
.wait1:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait1		;no, wait!
      move.b	d0,$fffffc06.w	;write to MIDI data register
.skipMidiStart:
      endif
      ;check pulses per quaternote
      move.l	_pCurrentSequence,a0

      if (MIDI_CLOCK_ENABLE==1)
      echo	"[amidi.s] MIDI clock generation enabled (PPQN/24)"
      ;divide pulse counter mod 24 if there is no remainder send byte directly to MIDI port 
      ;if result == 0 send midi status byte
      move.l	pulseCounter(a0),d1

      move.l	d1,d2
      moveq.l	#24,d3
      divu.w	d3,d2
      lsr.l	#4,d3
      and.l	#$0000FFFF,d3
      cmpi.b	#0,d3
      bne.s	.skipClock
      eor.w	#$00f,$ffff8240		;change 2nd color in palette (TODO: remove it in the final version)
  
.wait:
      btst	#1,$fffffc04.w			;is data register empty?
      beq.s	.wait				;no, wait!
      move.b	#MIDI_CLOCK_BYTE,$fffffc06.w	;write to MIDI data registers
      else
      echo	"[amidi.s] MIDI clock generation disabled"
      endif

.skipClock:
      eor.w	#$0f0,$ffff8240		;change 1st color in palette (TODO: remove it in the final version)
      ;jsr	_sequenceUpdate		;jump to sequence handler, sneaky bastard ;>

      move.l	_pCurrentSequence,a0
      move.l	pulseCounter(a0),d1 	;set counter

      addq.l	#1,d1			;increase counter
      move.l	d1,pulseCounter(a0)	;save it
.setInt:
      ;prepare next tick
      move.l    update,$120		;slap interrupt 
      move.b    _tbData,$fffffa21	;set data
      move.b    _tbMode,$fffffa1b	;div mode
      bset.b    #0,$fffffa07		;go!
      bset.b    #0,$fffffa13

      movem.l   (a7)+,d0-7/a0-6		;restore registers
      bclr.b	#0,$fffffa0f  		; finished!
      rte

      BSS
_pCurrentSequence:	ds.l	1      
_startPlaying:		ds.w	1
_MIDIdataEndPtr:	ds.l	1
_bTempoChanged:		ds.l	1

	align 4

;sSequence_t structure
   RSRESET
pSequenceName	rs.l	1	; NULL terminated string ptr
timeDivision	rs.l	1	; pulses per quater note(time division) 
eotThreshold	rs.l	1	; see define EOT_SILENCE_THRESHOLD 
accumDelta	rs.l	1	; accumulated delta counter 
pulseCounter	rs.l	1	; pulses per quaternote counter 
divider		rs.l	1	;
ubNumTracks	rs.w	1	; number of tracks 
ubActiveTrack	rs.w	1	; range 0-(ubNumTracks-1) tracks 
arTracks	rs.l	AMIDI_MAX_TRACKS; up to AMIDI_MAX_TRACKS (65536) tracks available

  RSRESET
