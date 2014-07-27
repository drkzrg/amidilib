
;    Copyright 2007-2014 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi sequence replay from custom structure

    include "timing/mfp_m68k.inc"
    include "common_m68k.inc"
    
    xdef replaySetupRoutine   ; custom sequence replay handler

; update callbacks
    xref _updateStepSingle ; our sequence update routine (single track)
    xref _updateStepMulti  ; our sequence update routine (multi track)

    xref update		   ; our installed interrupt routine

	TEXT
; ############## replaySetupRoutine
replaySetupRoutine:
        or.w    #$2700,sr               ; disable interrupts
        movem.l   d0-7/a0-6,-(a7)	; save registers

        move.l  stopTimerIntPtr,a0         ; stop timer
        jsr     (a0)

        move.l  updateRout,a0           ; update sequence state and send events / copy events to internal send buffer
        jsr 	(a0)

	if (IKBD_MIDI_SEND_DIRECT==1)
	echo	"[midiReplay.s] IKBD MIDI DATA SEND DIRECT ENABLED"

	move.w	#0,d1
	move.l 	#_MIDIsendBuffer,a0
	move.w	_MIDIbytesToSend,d1

        cmpi.w   #0,d1
        beq.s   .done       ;if 0 bytes do nothing
.send:      
      ; slap data to d0
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

        move.l  updateTimerIntPtr,a0   ;setup/update timer
        jsr   (a0)

.finish:
        move.l    finishTimerIntPtr,a0   ;finish timer
        jsr       (a0)

        movem.l   (a7)+,d0-7/a0-6	;restore registers
        move.w    #$2300,sr             ;enable interrupts
        rte


; installs / deinstalls MIDI replay (single/multitrack) on selected Timer Interrupt

; MFP_TiC     - installs / deinstalls replayRout on TiC
;               note: this non intrusive, steals original TiC vector and adds special jumptable
;               which launches music update routine if needed and after that runs old Timer C handler
; MFP_TiB     - standard custom vector
; MFP_TiA     - * unimplemented *

; #####################################################################################

_installReplayRout:
        movem.l	d0-d7/a0-a6,-(sp)
        move.l	$40(sp),d1  	; mode MFP interrupt mode
        move.l  $44(sp),d0  	; data MFP interrupt data
        move.l	$48(sp),d2      ; interrupt routine type multitrack / singletrack
        move.l  $4c(sp),d3      ; timer type

        move.w  #0,midiIntCounter

        move.w  d2,isMultitrackReplay
        move.w  d3,timerReplayType

        jsr	_super_on
        move.w	sr,-(sp)	;save status register
        or.w	#$0700,sr	;turn off all interupts

        move.b	d1,tMode  	;save parameters for later
        move.b	d0,tData

        move.l  #replaySetupRoutine, update

; check single / multitrack replay
        cmpi.w  #0,d2
        bne.s   .installMultiTrack
        move.l  #_updateStepSingle, updateRout
        bra.s   .checkIntType

.installMultiTrack:
        move.l  #_updateStepMulti, updateRout

.checkIntType:
        ; handle various timer versions
        cmpi.w  #MFP_TiB, d3
        bne.s   .checkTiC
  ; setup
        move.l	$120,oldVector
        move.l  #stopTiB, stopTimerIntPtr
        move.l  #updateTiB, updateTimerIntPtr
        move.l  #finishTiB, finishTimerIntPtr
        bra.s   .done

.checkTiC:
        cmpi.w  #MFP_TiC, d3
        bne.s   .error

        move.l	$114,oldVector            ;save TiC
        move.l  #stopTiC, stopTimerIntPtr
        move.l  #updateTiC, updateTimerIntPtr
        move.l  #finishTiC, finishTimerIntPtr
        move.l  #vectorTiC, $114.w
.done:
        move.l  stopTimerIntPtr,a0
        jsr     (a0)
        move.l    updateTimerIntPtr,a0
        jsr       (a0)
.error:
        move.w 	  (sp)+,sr 		;restore Status Register
        jsr	  _super_off
        movem.l (sp)+,d0-d7/a0-a6	;restore registers
        rts

_deinstallReplayRout:
        movem.l	  d0-d7/a0-a6,-(sp)
        jsr	_super_on

        move.w	sr,-(a7)		;save status register
        or.w	#$0700,sr

        move.l  stopTimerIntPtr,a0
        jsr     (a0)

        ;check timer type and deinstall
        move.w timerReplayType,d0

        cmpi.w  #MFP_TiB,d0
        bne.s   .checkTiC
        move.l  oldVector,$120.w
        bra.s   .done

 .checkTiC:
        cmpi.w  #MFP_TiC,d0
        bne.s   .error
        move.l  oldVector,$114.w

.done:
.error:
        move.w	(sp)+,sr	;restore Status Register
        jsr	_super_off
        movem.l (sp)+,d0-d7/a0-a6
        rts
