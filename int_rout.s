;    Copyright 2007-2010 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.
	
          xdef _installMIDIreplay
	  xdef _deinstallMIDIreplay
	  xdef _turnOffKeyclick
	  xdef	_installTA
	  xdef	_counter
	  xdef  _cA
	  
	  xdef super_on
	  xdef super_off

	  xref _oldTB
	  xdef _counter
	  xdef _tbMode
	  xdef _tbData
;external references
	  xref 		_hMidiEvent
	  
	  xdef		_g_pEventPtr
	  xdef		_iCurrentDelta

;MFP MK68901 Multi Function Periferal
MFP_STOP	equ 	%0000  ;Timer stop
MFP_DIV4	equ 	%0001  ;div 4
MFP_DIV10	equ	%0010  ;div 10
MFP_DIV16	equ     %0011  ;div 16
MFP_DIV50	equ	%0100  ;div 50
MFP_DIV64	equ	%0101  ;div 64
MFP_DIV100	equ	%0110  ;div 100
MFP_DIV200	equ	%0111  ;div 200
MFP_EC		equ	%1000  ;event count mode (TA/TB) 
MFP_DEL4	equ 	%1001  ;delay 4
MFP_DEL10	equ	%1010  ;delay 10
MFP_DEL16	equ     %1011  ;delay 16
MFP_DEL50	equ	%1100  ;delay 50
MFP_DEL64	equ	%1101  ;delay 64
MFP_DEL100	equ	%1110  ;delay 100
MFP_DEL200	equ	%1111  ;delay 200;

; installs MIDI replay on timer B
_installMIDIreplay:	 
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	  super_on
	move.w	sr,-(sp)	;save status register
        or.w	#$0700,sr	;turn off all interupts

	move.w	$54(sp),d1  ;mode
	move.w  $58(sp),d0  ;data
        move.b	d1,_tbMode  ;save parameters for later
	move.b	d0,_tbData

	clr.b     $fffffa1b		;turn off tb
	move.l	  #0,_counter		;init counter 
	move.l	  $120,_oldTB	
	move.l    #tickCounter,$120	;slap interrupt
	
	move.b    d0,$fffffa21		;put data 
	move.b    d1,$fffffa1b		;put mode 
	bset.b    #0,$fffffa07
	bset.b    #0,$fffffa13
	
	move.w 	  (sp)+,sr 		;restore Status Register
	bsr.w	  super_off
	movem.l (sp)+,d0-d7/a0-a6		;restore registers
	rts

; deinstalls MIDI replay on timer B 
_deinstallMIDIreplay:
	movem.l	  d0-d7/a0-a6,-(sp)
	bsr.w	super_on

	move.w	sr,-(a7)		;save status register
	or.w	#$0700,sr

	clr.b     $fffffa1b	;turn off tb
	move.l	 _oldTB,$120	;save old tb
	
        move.w	(sp)+,sr		;restore Status Register

	bsr.w	super_off
	movem.l (sp)+,d0-d7/a0-a6
	rts

_turnOffKeyclick:
      bsr.w	super_on
      bclr	#0,$484.w

      bsr.w	super_off
      rts

_installTA:
	bsr.w	super_on
	pea	_tiA
	move.w	#59,-(sp)
	move.w	#6,-(sp)
	move.w	#0,-(sp)
	move.w	#$1f,-(sp)
	trap	#14
;enable interrupt
	move.w	#13,-(sp)
	move.w	#$1b,-(sp)
	trap	#14
	addq.l	#4,sp
	lea	$c(sp),sp
	bsr.w	super_off
	rts

tickCounter:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240

;<----- insert your code here ----- >
	
	;send events we are fucking lazy today, so everything will be done in C :P
	
	;check if event is set tempo
	;if yes set new values in _tbData,_tbMode
	;get event 
	
	bsr.w	_hMidiEvent			;by two pointers to the next or previous event
 
        cmp.w	#$ffff,_counter			;if max int we have reached the end of track
	beq.s	.finish
        ;check if it is null or eot
	;;cmpi.b	#,d1
	;firstly check if pEvent->delta==iCurrentDelta
	;if yes send midi event(or all events with current delta?) 
	
        ;adjust pEvent pointer (next event OR the first one (pEvent->delta!=iCurrentDelta)) 
	;iCurrentDelta=pEvent->delta

	moveq.l	#30,d0		;init counter with next delta value(iCurrentDelta)	
	move.l	d0,(a0)
		

.exit:
;<----- insert your code here ----- >
        move.l    #tickCounter,$120	;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
.finish:	
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	bclr.b	  #0,$fffffa0f  	; finished!
	rte                 		; return from timer

_tiA:
	movem.l   d0-7/a0-6,-(a7)	;save registers
	
	lea.l	_cA,a0
   	move.l	(a0),d0
	addq.l	#1,d0
	move.l	d0,(a0)

	eor.w	  #$00f,$ffff8240
	bclr.b	  #5,$fffffa0f  ; finished!
	movem.l   (a7)+,d0-7/a0-6	;restore registers
	rte
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
	even
_oldTB:	  	ds.l	1
old_ssp:	ds.l	1
_counter:	ds.l	1
_cA: 		ds.l 	1
_g_pEventPtr:	ds.l 	1
_iCurrentDelta: ds.l 	1

	even
_tbMode:	ds.b	1
	even
_tbData:	ds.b	1

;================================== structs ==========================================================================================
;offsets has to be like in AMIDISEQ.H
AMIDI_MAX_TRACKS	equ	16

; play mode flags
S_PLAY_ONCE 	equ	$00
S_PLAY_LOOP 	equ	$02
S_PLAY_RANDOM 	equ	$04

;play state flags
S_STOPPED 	equ	$00
S_PLAYING	equ	$02
S_PAUSED 	equ	$04


	RSRESET ;sEventList
seNbOfItems	rs.l	1
seEventList	rs.l	1

	RSRESET		;eventItem
eipEventBlock  	rs.b	17	; EventBlock_t
eipPrev		rs.l	1	;ptr
eipNext		rs.l	1	;ptr
 
      RSRESET	;EventBlock_t
ebDT		rs.l	1	; event delta time 
ebEventType	rs.b	1	; event type 
ebInfoBlock	rs.l 	2 	; function info block (EventInfoBlock_t) 
ebDataPtr	rs.l 	1	; pointer to event data of sEventInfoBlock_t.size * 1 byte (U8)

      RSRESET	;EventInfoBlock_t
eibSize 	rs.l	1	;size of command string in bytes
eibFuncPtr 	rs.l	1	;pointer to event handler 

	RSRESET ;sTrackState
tsCurrentPos	rs.l	1	; current delta position, 0 at start 
tsVolume	rs.b	1	;sequence current track volume
tsTimeStep	rs.l	1	;sequence current update time step
tsTrackTempo	rs.l	1	;miliseconds per quarter note, default 500 000ms

      RSRESET	;Track_t
ttInstrumentName rs.l	1	;NULL terminated string with instrument name, track data and other text from MIDI meta events .. 
ttCurrTrackState  rs.b   13	;current track state 
ttTrkEventList	rs.l	2	;track event list 

      RSRESET ;SequenceState_t
ssVolume	rs.b	1	; sequence master volume
ssActiveTrack	rs.b	1	; sets the active track, by default 0 
ssPlayState	rs.b	1	; indicates if sequence is currently being played or not: S_STOPPED (stopped), S_PLAYING (playing),S_PAUSED (paused)sequence is being played / FALSE */          
ssPlayModeState rs.b	1	; current play mode (loop, play_once) 
sstimeElapsed 	rs.l	1	; TODO: move time/timeElapsed to global space (?)
ssTime	    	rs.l	1	;

    RSRESET ;Sequence_t internal midi data storage format 
stSequenceName	rs.l	1	; NULL terminated string 
stNumTracks	rs.b	1	; number of tracks 
stTimeDivision	rs.w	1	; PPQN 
stTracks	rs.l	AMIDI_MAX_TRACKS ; up to AMIDI_MAX_TRACKS tracks available 
stCurrentState	rs.l	3	; current sequence state 
  RSRESET
