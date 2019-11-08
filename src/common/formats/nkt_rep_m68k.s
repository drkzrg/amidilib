;    Copyright 2019 Pawel Goralski
;    
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi sequence replay from custom NKT structure

    include "timing/mfp_m68k.inc"
    include "common_m68k.inc"
    include "m68k_defs.inc"

	xref  update			; function pointer to current interrupt routine
	xref  updateStepRout		; function pointer holding current replay routine address

	xref _updateStepNkt		; update step for single track replay
	xref _updateStepNktMt		; update step for multitrack replay


        xdef _NktInstallReplayRoutNoTimers	; sets Nkt replay interrupt routine (single / multitrack) without setting timers

	xdef _NktInstallReplayRout	; initialises Nkt replay interrupt routine (single / multitrack) on selected timer
        xdef _NktDeinstallReplayRout	; removes Nkt replay routine from system
	xdef _NktMidiUpdateHook
	xdef _clearMidiOutputBuffer	; clears MidiOutputBuffer
        even

replayNkt:
        movem.l   d0-7/a0-6,-(a7)	; save registers

        move.l	updateStepRout,d0	; check if anything is installed
	tst.l	d0
	beq.s	.nothingToDo

        move.l	d0,a0

	jsr 	(a0)			; update sequence state and send events / copy events to internal send buffer

	if	(TX_ENABLE==0)

        if (IKBD_MIDI_SEND_DIRECT==1)
	echo	"[nkt_rep_m68k.s] IKBD MIDI DATA SEND DIRECT ENABLED"

        move.w	#0,d1
        move.l 	#_MIDIsendBuffer,a0
        move.w	_MIDIbytesToSend,d1

	move.w  sr,-(a7)
	or.w	#$2300,sr		; disable some interrupts


        cmpi.w   #0,d1
        beq.s   .done       ;if 0 bytes do nothing

        ifne (__VASM & m68000)
        echo "[nkt_rep_m68k.s] Midi send 68000 target variant"

.send:
      ; slap data to d0
      move.w	(a0)+,d0  ; get word

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

      else
      echo "[nkt_rep_m68k.s] Midi send 68030 target variant"

.send:
      ; slap data to d0
      move.b	(a0)+,d0  ; get byte

.wait:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait		;no, wait!
      move.b	d0,$fffffc06.w	;write to MIDI data register

      subq.w	#1,d1
      cmpi.w	#0,d1
      beq.s	.done

      bra.s	.send

      endif  ;end 68030>= part

.done:
        move.w	#0,_MIDIbytesToSend
	move.w    (a7)+,sr             ;restore sr

	else
	echo	"[nkt_rep_m68k.s] IKBD MIDI DATA SEND DIRECT DISABLED"
        endif

	else
	echo	"[nkt_rep_m68k.s] ACIA WRITE SKIP"
	endif


	;# interrupt will be called automatically next time
	;# no need to reset if AUTO_INT_ENABLE==1, enabled AER mode

	if(AUTO_INT_ENABLE==1)
	echo	"[nkt_rep_m68k.s] AEI enabled"
	else
	echo	"[nkt_rep_m68k.s] AEI disabled"
	move.l  updateTimerIntPtr,a0	 ;setup/update timer
	jsr   (a0)

	move.l    finishTimerIntPtr,a0   ;signal end of and interrupt whatever it might be
	jsr   (a0)
	endif

.nothingToDo:

        movem.l   (a7)+,d0-7/a0-6	;restore registers
	rte


_NktMidiUpdateHook:
	movem.l   d0-7/a0-6,-(a7)	; save registers

        move.l	updateStepRout,d0	; check if anything is installed
	tst.l	d0
	beq.s	.nothingToDo

        move.l	d0,a0
	jsr 	(a0)			; update sequence state and send events / copy events to internal send buffer

	if	(TX_ENABLE==0)


	if (IKBD_MIDI_SEND_DIRECT==1)
	echo	"[nkt_rep_m68k.s] IKBD MIDI DATA SEND DIRECT ENABLED"

	move.w	#0,d1
	move.l 	#_MIDIsendBuffer,a0
	move.w	_MIDIbytesToSend,d1

	move.w  sr,-(a7)
	or.w	#$2300,sr		; disable interrupts, leave ikbd

	cmpi.w   #0,d1
	beq.s   .done       ;if 0 bytes do nothing

	ifne (__VASM & m68000)
	echo "[nkt_rep_m68k.s] Midi send 68000 target variant"

.send:
      ; slap data to d0
      move.w	(a0)+,d0  ; get word

      move.w	d0,d2		; make copy
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

      else
      echo "[nkt_rep_m68k.s] Midi send 68030 target variant"

.send:
      ; slap data to d0
      move.b	(a0),d0  ; get byte
      clr.b	(a0)+ 	 ; clear it

.wait:
      btst	#1,$fffffc04.w	;is data register empty?
      beq.s	.wait		;no, wait!
      move.b	d0,$fffffc06.w	;write to MIDI data register

      subq.w	#1,d1
      cmpi.w	#0,d1
      beq.s	.done

      bra.s	.send

      endif  ;end 68030>= part
.done:
	move.w	#0,_MIDIbytesToSend
	else
	echo	"[nkt_rep_m68k.s] IKBD MIDI DATA SEND DIRECT DISABLED"
	endif

	move.w    (a7)+,sr              ; restore sr

	else
	echo	"[nkt_rep_m68k.s] ACIA WRITE SKIP"
	endif

.nothingToDo:

	movem.l   (a7)+,d0-7/a0-6	; restore registers

	rts


_NktInstallReplayRoutNoTimers:
    movem.l	d0-d7/a0-a6,-(sp)
    move.w	sr,-(sp)	;save status register

    move.w  #0,midiIntCounter

; set adequate update step routine depending on replay type

    move.w	_isMultitrackReplay,d0
    tst.w	d0
    beq.s	.isSingleTrack
    move.l	#_updateStepNktMt, updateStepRout
    bra.s	.done
.isSingleTrack:
    move.l	#_updateStepNkt, updateStepRout
.done:
    move.w 	  (sp)+,sr 		;restore Status Register
    movem.l (sp)+,d0-d7/a0-a6	;restore registers
    rts

; installs / deinstalls MIDI replay (single/multitrack) on selected Timer Interrupt

; #####################################################################################

_NktInstallReplayRout:
        movem.l	d0-d7/a0-a6,-(sp)
	move.w	sr,-(sp)	;save status register

	move.w  #0,midiIntCounter

; set adequate update step routine depending on replay type

	move.w	_isMultitrackReplay,d0
	tst.w	d0
	beq.s	.isSingleTrack
	move.l	#_updateStepNktMt, updateStepRout
	bra.s	.done

.isSingleTrack:
	move.l	#_updateStepNkt, updateStepRout
.done:

        or.w	#$0700,sr	;turn off all interupts

        if(AUTO_INT_ENABLE==1)
	echo	"[nkt_rep_m68k.s] AUTO INT ENABLED"
	bclr.b	#3,$fffffa17.w
	endif

	move.l  #replayNkt, update	  ; set interrupt update function

        move.l	$114,oldVector            ; save TiC
        move.l  #stopTiC, stopTimerIntPtr
        move.l  #updateTiC, updateTimerIntPtr

	if(AUTO_INT_ENABLE==1)
	echo	"[nkt_rep_m68k.s] AUTO INT ENABLED"
	move.l  #0, finishTimerIntPtr
	else
	move.l  #finishTiC, finishTimerIntPtr
	endif

	move.l  #vectorTiC, $114.w	; install custom vector

        move.l    stopTimerIntPtr,a0
        jsr       (a0)
        move.l    updateTimerIntPtr,a0
        jsr       (a0)
.error:
        move.w 	(sp)+,sr 		;restore Status Register
        movem.l (sp)+,d0-d7/a0-a6	;restore registers
        rts

_NktDeinstallReplayRout:
        movem.l	  d0-d7/a0-a6,-(sp)
	move.w	sr,-(sp)		;save status register

	or.w	#$0700,sr

	if(AUTO_INT_ENABLE==1)
	echo	"[nkt_rep_m68k.s] AUTO INT ENABLED"
	bset.b	#3,$fffffa17.w		;reenable software end int
	endif

	move.l  stopTimerIntPtr,a0
	jsr     (a0)

	move.l    oldVector,$114.w
	move.b    #246,$fffffa21.w	; set data
	ori.b     #80,$fffffa1b.w	; div mode
	bset.b    #5,$fffffa09.w        ; enable TiC
	bset.b    #5,$fffffa15.w        ; set interrupt mask B

	move.l	  #0, updateStepRout	; clear update step rout

	move.w 	  (sp)+,sr 		;restore Status Register
        movem.l (sp)+,d0-d7/a0-a6
        rts

_clearMidiOutputBuffer:
	movem.l	  d0-d7/a0-a6,-(sp)
	move.w	sr,-(sp)

	or.w	#0700,sr

	move.w	#0,_MIDIbytesToSend

	move.l 	#_MIDIsendBuffer,a0
	add.l	#1024*32,a0

	move.l	#630-1,d7

	move.l	#0,d0
	move.l	#0,d1
	move.l	#0,d2
	move.l	#0,d3
	move.l	#0,d4
	move.l	#0,d5
	move.l	#0,d6

	move.l	#0,a1
	move.l	#0,a2
	move.l	#0,a3
	move.l	#0,a4
	move.l	#0,a5
	move.l	#0,a6
.cpy:
	movem.l	d0-d6/a1-a6,-(a0)
	dbra d7,.cpy

	move.w	(sp)+,sr
	movem.l (sp)+,d0-d7/a0-a6
	rts
