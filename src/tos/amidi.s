;================================================
;    Copyright 2007-2011 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.
;================================================

;constants
AMIDI_MAX_TRACKS	equ	65536
MIDI_CLOCK_ENABLE	equ	1
MIDI_SEND_BUFFER_ENABLE	equ	0	;enables code part which sends MIDI data
					;from internal buffer
					;once per frame
MIDI_SENDBUFFER_SIZE 	equ	8000

MIDI_CLOCK_BYTE		equ 	$F8	;one byte MIDI realtime message (MIDI CLOCK)
MIDI_START		equ 	$FA	;one byte MIDI realtime message (starts MIDI clock sync)
MIDI_STOP		equ 	$FC	;one byte MIDI realtime message (stops MIDI clock sync)

;export symbols
    xdef _super_on		; self explanatory 
    xdef _super_off		;
    xdef _turnOffKeyclick	;
    xdef _installReplayRout	;initialises replay interrupt TB routine and prepares data
    xdef _deinstallReplayRout	;removes replay routine from system 
    xdef _pCurrentSequence	;current sequence pointer
    
    xdef _MIDIsendBuffer	;buffer with data to send
    xdef _MIDIbufferReady	;if 0> buffer is ready for sending data, otherwise we want ti fill it
    
    xdef _tbData
    xdef _tbMode
    xdef _amMidiSendIKBD	;bypass of Atari XBIOS, writes midi data directly to IKBD
    xdef _startPlaying		;to let us now if there is an end of sequence
				;so MIDI beat 0 is send only once
;import symbols
    xref _sequenceUpdate


_installReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	  _super_on
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
	bsr.w	  _super_off
	movem.l (sp)+,d0-d7/a0-a6		;restore registers
	rts

update:
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
      echo	"[VASM]***************** MIDI clock generation enabled (PPQN/24)"
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
      echo	"[VASM]***************** MIDI clock generation disabled"
      endif

.skipClock:
      eor.w	#$0f0,$ffff8240		;change 1st color in palette (TODO: remove it in the final version)
      jsr	_sequenceUpdate		;jump to sequence handler, sneaky bastard ;>

      if (MIDI_SEND_BUFFER_ENABLE==1)
      ;send the whole buffer if not empty
      move.l	_MIDIdataEndPtr,d1  ;count
      movea.l  	#_MIDIsendBuffer,a0  ;buffer
  
.loop:
      move.l	a0,d2
      cmp.l	d1,d2
      beq.s	.done
      cmpi.l	#0,(a0)	
      beq.s	.done
      
      ;slap data to d0
      move.b	(a0)+,d0
.wait:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait		;no, wait!
      move.b	d0,$fffffc06.w	;write to MIDI data register

      subq.l	#1,d1
      bra.s	.loop
.done:
      movea.l	#_MIDIsendBuffer,_MIDIdataEndPtr ; set endptr to the beginning
						 ; of the buffer
      endif
      move.l	_pCurrentSequence,a0
      move.l	pulseCounter(a0),d1 	;set counter

      addq.l	#1,d1			;increase counter
      move.l	d1,pulseCounter(a0)	;save it
.setInt:
      ;prepare next tick
      move.l    #update,$120		;slap interrupt 
      move.b    _tbData,$fffffa21	;set data
      move.b    _tbMode,$fffffa1b	;div mode
      bset.b    #0,$fffffa07		;go!
      bset.b    #0,$fffffa13

      movem.l   (a7)+,d0-7/a0-6		;restore registers
      bclr.b	#0,$fffffa0f  		; finished!
      rte

; deinstalls MIDI replay on timer B 
_deinstallReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	_super_on

	move.w	sr,-(a7)		;save status register
	or.w	#$0700,sr

	clr.b     $fffffa1b	;turn off tb
	move.l	 _oldTB,$120	;save old tb
	
        move.w	(sp)+,sr	;restore Status Register

	bsr.w	_super_off
	movem.l (sp)+,d0-d7/a0-a6
	rts

_turnOffKeyclick:
      bsr.w	_super_on
      bclr	#0,$484.w

      bsr.w	_super_off
      rts

_amMidiSendIKBD:
      movem.l	d0-d7/a0-a6,-(sp)
      bsr.w	_super_on

      move.l	$42(sp),d1  ;count
      ;move.l  	$46(sp),a0  ;buffer
.loop:      
      cmpi.l	#0,d1	
      beq.s	.done
      
      ;slap data to d0
      move.b	(a0)+,d0
.wait:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait		;no, wait!
      move.b	d0,$fffffc06.w	;write to MIDI data register

      subq.l	#1,d1
      bra.s	.loop
.done:
      bsr.w	_super_off
      movem.l (sp)+,d0-d7/a0-a6
      rts

;enter supervisor mode
_super_on:
	movem.l	d0/a0,-(sp)
	clr.l	-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	move.l	d0,old_ssp
	movem.l	(sp)+,d0/a0
	RTS

;leave supervisor mode
_super_off:
	movem.l	d0/a0,-(sp)
	move.l	old_ssp,-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	movem.l	(sp)+,d0/a0 
	RTS

	bss
	align 2
old_ssp:		ds.l	1
	align 2
_pCurrentSequence:	ds.l	1
	align 2
_oldTB:			ds.l	1
	align 2
_tbData:		ds.b	1
	align 2
_tbMode:		ds.b	1
	align 2		
_startPlaying:		ds.w	1
	align 2
_MIDIdataEndPtr:	ds.l	1
	align 2
_MIDIsendBuffer:	ds.b	MIDI_SENDBUFFER_SIZE
_MIDIbufferReady:	ds.w	1
_MIDIbufferPos:		ds.w	1	

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








