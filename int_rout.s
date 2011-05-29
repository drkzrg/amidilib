;    Copyright 2007-2010 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

S_PLAY_ONCE 	equ	$00
S_PLAY_LOOP 	equ	$02
S_PLAY_RANDOM 	equ	$04

S_STOPPED 	equ	$00
S_PLAYING 	equ	$02
S_PAUSED 	equ	$04

SAVEPTR		equ	$4A2	;to safely call BIOS/XBIOS from interrupt
SAVAMT		equ	$46	;but only the functions which doesn't access disc
				;TODO: make workaround form MiNT/MultiTOS

AMIDI_MAX_TRACKS	equ	16


;=========================================
       xdef _installReplayRout	;initialises replay interrupt TB routine and prepares data
       xdef _deinstallReplayRout	;removes replay routine from system 
       xdef _defaultPlayMode
	   
       xdef _oldTB		;saved old Timer B vector
       xdef _counter	;ticks counted in quaternote span
	   
       xref super_on		; self explanatory 
      xref super_off		;
      xref _turnOffKeyclick  
	 	  
;external references
	  xref _sequenceUpdate
	  xref _am_allNotesOff
	  xref _getMFPTimerSettings
	  xref ___udivsi3
    

_installReplayRout:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	  super_on
	move.w	sr,-(sp)	;save status register
	or.w	#$0700,sr	;turn off all interupts

	move.l	arTracks(a0),a1 ;get current tempo, current slot in the sequence,
	move.l	currentTempo(a1),d0
	move.l	d0,_currentTempo	;save current tempo

	move.l	$42(sp),d1  ;mode
        move.l  $46(sp),d0  ;data
        
	move.b	d1,_tbMode  ;save parameters for later
	move.b	d0,_tbData

	clr.b     $fffffa1b		;turn off tb
	move.l	  #0,_counter		;init counter 
	move.l	  #0,_elapsedDelta
	move.l	  #S_PLAY_ONCE,_defaultPlayMode
	
	move.l	  $120,_oldTB	
	move.l    #update2,$120		;slap interrupt
	
	move.b    d0,$fffffa21		;put data 
	move.b    d1,$fffffa1b		;put mode 
	bset.b    #0,$fffffa07
	bset.b    #0,$fffffa13
	
	move.w 	  (sp)+,sr 		;restore Status Register
	bsr.w	  super_off
	movem.l (sp)+,d0-d7/a0-a6		;restore registers
	rts

update2:
      movem.l   d0-7/a0-6,-(a7)	;save registers
      ;move.w	sr,-(sp)	;save status register
      ;or.w	#$0700,sr	;turn off all interupts 

      clr.b     $fffffa1b
      eor.w	  #$0f0,$ffff8240	;change 1st color in palette (TODO: remove it in the final version)

      jsr	_sequenceUpdate		;sneaky bastard ;>

      ;prepare next tick
      move.l    #update2,$120		;slap interrupt 
      move.b    _tbData,$fffffa21	;set data
      move.b    _tbMode,$fffffa1b	;div mode
      bset.b    #0,$fffffa07		;go!
      bset.b    #0,$fffffa13

      ;move.w 	  (sp)+,sr 		;restore Status Register
      movem.l   (a7)+,d0-7/a0-6		;restore registers
      bclr.b	  #0,$fffffa0f  	; finished!
      rte

update:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	move.w	sr,-(sp)		;save status register
	or.w	#$0700,sr		;turn off all interupts

	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240	;change 1st color in palette (TODO: remove it in the final version)

	move.l	_currentSeqPtr,a0
	
	move.l	arTracks(a0),a1 	;get current tempo, current slot in the sequence,
	move.l	currentState(a1),a1 
	move.l	currentTempo(a1),d0
	
	;check if change of current tempo has occurred
	;if yes then calculate new tick frequency and get new data/divider
	;which will be set at the end of update rout
	move.l	_currentTempo,d2
	cmp.l	d0,d2
	beq.s  .skipTempo

;calculate frequency tempo / resolution in PPQN
	movem.l	d0-d7/a0-a6,-(sp)
	
	move.l arTracks(a0),a1 ;get current tempo, current slot in the sequence,
	move.l currentTempo(a1),d0 ;get tempo from current/active track
	move.l timeDivision(a0),d1 ;get time division
	
	move.l d1,-(sp)		
	move.l d0,-(sp)
	
	jsr ___udivsi3		 ;remove this, and replace with FPU/non-FPU versions
	addq.l #8,sp		 ;calculate new frequency
	
	;get result from d0, desired tick frequency 
	;put frequency, addr to mode/data
	move.l	#timerData,-(sp)
	move.l	#timerMode,-(sp)
	move.l d0,-(sp)	
	
	jsr _getMFPTimerSettings
	lea (12,sp),sp
	
    move.l timerMode,d0
    move.l	timerData,d1
	or.l	d0,d1
    cmp.l	#0,d1	;if data or mode 0 skip change
	beq.s	.restore

    move.b	d0,_tbMode
    move.b	d1,_tbData
      
	;get MFP mode and data
.restore:
	movem.l	(sp)+,d0-d7/a0-a6
	
.skipTempo:
	move.l	arTracks(a0),a1     	    ;ptr to TRACK
        move.l  currentState(a1),a1
	move.l	playState(a1),d2	    ;current state of sequence

        cmp.l	#S_STOPPED,d2		    ;is sequence stopped?
	bne.s	.paused		    	    ;not equal check if we have pause
        
	;we have stop so we have to reset all counters and set
	;current track to 0 index
	move.l	#0,_counter
	move.l	#0,_elapsedDelta

	;update current track state 
        move.l	pStart(a1),pCurrent(a1)
	
        ;we go straight to exit, nothing to do
	bra  .exit

.paused:
	cmpi.l	#S_PAUSED,d2		;is sequence paused?
	bne.s	.handleSeq
				;we go straight to the exit, nothing to do
	bra.w  .exit

.handleSeq:
	; here we handle our note from event list 
	; if internal tick counter has reached our current event delta value
	; we play note read from the sequence and we set next event as current
	; firstly we check if we are on the end of sequence (current event is null)

	cmpi.l	#0,pCurrent(a1)		;a1 current track state
	beq.w	.endSeqHandle
	;nope, so
	;check _counter if is equal to PPQN-1
	;if yes, increase delta
	;if no, only increase _counter

	move.l _counter,d6
	move.l	timeDivision(a0),d7  ;get timeDivision from current sequence(a0)
	subq	#1,d7
	cmp.l	d7,d6
        bne.s .increaseCounter
	;now we can increase delta 
	
	move.l	_elapsedDelta,d6
	; now,
	; a0 - current sequence ptr
	; a1 - current track state ptr
	
        move.l	pCurrent(a1),a2
        
	;we have pointer to current sEventList in a2
	move.l	eventBlock(a2),a3
	move.l	delta(a3),d3	;get delta
	
	cmp.l	d6,d3
	beq.s	.sendNote
	
	;increase elapsed delta
	move.l	_elapsedDelta,d3
	addq.l	#1,d3
	move.l	d3,_elapsedDelta
	;reset tick counter _counter
	move.l	#0,_counter
	;exit

	bra	.exit
.sendNote:
	; a0 - current sequence ptr
	; a1 - current track state ptr
	; a2 - current sEventList
	; a3 - current eventBlock

	;sub.l	#savamt,savptr
	movem.l	d0-d2/a0-a3,-(sp)
	
        ;TODO trigger handler for current event
	move.l	dataPtr(a3),-(sp)
	move.l	infoBlock(a3),a3
	addq.l	#4,a3
	jsr	(a3)
	;update current event pointer to the next event
	move.l	pNext(a2),pCurrent(a1)	;get next event and save it to current one

	movem.l	(sp)+,d0-d2/a0-a3

	move.l	#0,_elapsedDelta
	move.l	#0,_counter

	;TODO set next event 
   
	bra.s	.exit

.increaseCounter:
	; increase internal tick counter
        move.l	_counter,d0
	addq.l	#1,d0
	move.l	d0,_counter

	bra.s	.exit

.endSeqHandle:
	; if yest then we handle following modes
	; PLAY_ONCE=1, PLAY_LOOP=2, PLAY_RANDOM=3(TODO)
	; a1 should contain our sequence pointer
	;move.l	state(a0),d0
	
;reset current indexes and delta times ticks
	   
	move.l	#0,_counter
	move.l	#0,_elapsedDelta
	
	cmp.l	#S_PLAY_LOOP,d0
	beq.s	.exit
	;change state to STOP, turn off all instruments 
	
	movem.l	d0-d2/a0-a2,-(sp)
	pea 16.w
	jsr _am_allNotesOff
	addq.l #4,sp
	movem.l	(sp)+,d0-d2/a0-a2
	
	;move.l	#S_STOPPED,state(a0)		;set state to STOP
.exit:
	;prepare next tick
        move.l    #update,$120		;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
.finish:	
	move.w 	  (sp)+,sr 		;restore Status Register
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr.b	  #0,$fffffa0f  	; finished!
	rte                 		; return from timer


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

_currentSeqPtr:		ds.l	1
_elapsedDelta:		ds.l	1
_defaultPlayMode:	ds.l	1
_currentTempo:		ds.l	1
timerData:		ds.l	1
timerMode:		ds.l	1
_counter:		ds.l	1
_oldTB:			ds.l	1
_tbData:		ds.b	1
_tbMode:		ds.b	1

;================================== structs ==========================================================================================
;offsets has to be like in AMIDISEQ.H

;sequence structures as defined in amidiseq.h
 
  RSRESET	;EventInfoBlock_t
cmdSize 	rs.l	1	;size of command string in bytes
cmdFuncPtr 	rs.l	1	;pointer to event handler 


  RSRESET	;EventBlock_t
delta		rs.l	1	; event delta time  
infoBlock	rs.l 	2 	; function info block (EventInfoBlock_t) 
dataPtr		rs.l 	1	; pointer to event data of sEventInfoBlock_t.size * 1 byte (U8)
eventType	rs.b	1	; event type
fillpad		rs.b	1	; filler

; sEventList
  RSRESET
eventBlock	rs.b	18;
pPrev		rs.l	1
pNext		rs.l	1

;SequenceState_t
  RSRESET
currentTempo		rs.l	1	; quaternote duration in ms, 500ms default
playState		rs.l	1	; STOP, PLAY, PAUSED	
playMode		rs.l	1	; current play mode (loop, play_once, random) 
pStart			rs.l	1	;first event pointer
pCurrent		rs.l	1       ;current event pointer  

;Track_t
  RSRESET
pInstrumentName		rs.l	1	; NULL terminated string with instrument name, track data and other text from MIDI meta events .. 
currentState		rs.l	4	; current sequence state 
trkEventList		rs.l	2	; track event list 

;Sequence_t
  RSRESET
pSequenceName 	rs.l	1	; NULL terminated string 
timeDivision	rs.l	1	; pulses per quater note(time division)
ubNumTracks 	rs.b	1	; number of tracks 
ubDummy		rs.b	1	; filler
ubActiveTrack	rs.b	1	; currently active track
ubDummy1	rs.b	1	; filler
arTracks	rs.l	16	; up to AMIDI_MAX_TRACKS (16) tracks available 


