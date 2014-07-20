;================================================
;    Copyright 2007-2014 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.
;================================================

  include "common_m68k.inc"
  include "timing/mfp_m68k.inc"

; installs / deinstalls interrupt routine on timer B (used in demos, probably this pair should be removed at some point)

_installReplayRoutGeneric:
        movem.l	  d0-d7/a0-a6,-(sp)
        move.l	$40(sp),d1  	; mode
        move.l  $44(sp),d0  	; data
        move.l	$48(sp),update  ; interrupt routine ptr

        bsr.w	  _super_on
        move.w	sr,-(sp)	;save status register
        or.w	#$0700,sr	;turn off all interupts

        move.b	d1,tMode  	;save parameters for later
        move.b	d0,tData

        clr.b     $fffffa1b	;turn off tb

        move.l	  $120,oldVector
        move.l    update,$120		;slap interrupt

        move.b    d0,$fffffa21		;put data
        move.b    d1,$fffffa1b		;put mode
        bset.b    #0,$fffffa07
        bset.b    #0,$fffffa13

        move.w 	  (sp)+,sr 		;restore Status Register
        bsr.w	  _super_off
        movem.l (sp)+,d0-d7/a0-a6	;restore registers
        rts

_deinstallReplayRoutGeneric:
        movem.l	  d0-d7/a0-a6,-(sp)
        bsr.w	_super_on

        move.w	sr,-(a7)		;save status register
        or.w	#$0700,sr

        clr.b     $fffffa1b	;turn off tb
        move.l	 oldVector,$120	;save old tb

        move.w	(sp)+,sr	;restore Status Register

        bsr.w	_super_off
        movem.l (sp)+,d0-d7/a0-a6
        rts

;custom TiC vector
; calls standard vector
vectorTiC:

;check if we want to update
        jsr update

        jsr oldVector
        rte



; ####################################################################
_turnOffKeyclick:
      bsr.w	_super_on
      bclr	#0,$484.w

      bsr.w	_super_off
      rts

;sends midi data directly to ikbd, plain m68k friendly
_flushMidiSendBuffer:
      movem.l	d0-d1/a0,-(sp)
      bsr.w	_super_on
      
      move.w  #0,d1
      move.l  #_MIDIsendBuffer,a0
      move.w _MIDIbytesToSend,d1

      cmpi.w	#0,d1	
      beq.s	.done		;nothing to be done

.send:      
      ;slap data to d0
      move.w	(a0)+,d0	;get word
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
      move.w 	#0,_MIDIbytesToSend
      bsr.w	_super_off
      
      movem.l (sp)+,d0-d1/a0
      rts

 ;enter supervisor mode
_super_on:
	movem.l	d0-7/a0-a6,-(sp)
	clr.l	-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	move.l	d0,old_ssp
	movem.l	(sp)+,d0-7/a0-a6
	RTS

;leave supervisor mode
_super_off:
	movem.l	d0-7/a0-a6,-(sp)
	move.l	old_ssp,-(sp)
	move.w	#$20,-(sp)
	trap	#1
	addq.l	#6,sp
	movem.l	(sp)+,d0-7/a0-a6 
	RTS

;########################## redirect output to serial		
; redirect to serial
_redirectToSerial:
	move.w #2,-(sp)
	move.w #1,-(sp)
	move.w #$46,-(sp)
	trap #1
	addq.l #6,sp
	RTS
	
	  BSS
          align 4
old_ssp:		ds.l	1
update:			ds.l	1

; Timer data
oldVector:		ds.l	1
tData:                  ds.b	1       ; current timer data
dummy1:			ds.b	1	
tMode:                  ds.b	1       ; current timer mode
dummy2:			ds.b	1

updateRout:             ds.l    1       ; pointer to update routine (single / multitrack)
stopTimerPtr:           ds.l    1       ;
updateTimerPtr:         ds.l    1
isMultitrackReplay:     ds.w    1       ; flag indicates if multitrack / single track replay is installed
timerReplayType:        ds.w    1       ; currently installed midi sequence handler type (TiB/TiC etc.)

; only for TimerC based replay,
midiIntCounter:        ds.w    1        ; counts 200hz ticks, at certain point value is reset and midi update function is called

_MIDIbytesToSend:	ds.w	1	; nb of bytes to send
_midiOutEnabled:	ds.l	1	;
_ymOutEnabled:		ds.l	1	;
_bTempoChanged:		ds.l	1
_MIDIsendBuffer:	ds.b	MIDI_SENDBUFFER_SIZE



