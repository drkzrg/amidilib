;    Copyright 2014 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi sequence replay from custom NKT structure

    include "timing/mfp_m68k.inc"
    include "common_m68k.inc"

        xref    update
        xref    _updateStepNkt

        xdef _NktInstallReplayRout	; initialises Nkt replay interrupt routine (single / multitrack) on selected timer
        xdef _NktDeinstallReplayRout	; removes Nkt replay routine from system

        even
replayNktTC:
        movem.l d0-7/a0-6,-(sp)         ; save registers
        move.w  sr,-(sp)                ; save status register
        or.w    #$0700,sr               ; disable interrupts

;       jump to the old interrupt routine afterwards
;       send midi status byte once per 1ms
;       assume 200Hz update for TC (?)

        andi.b  #$0F,$fffffa1d.w        ; disable TC
        jsr 	_updateStepNkt          ; update sequence state and send events / copy events to internal send buffer

        if (IKBD_MIDI_SEND_DIRECT==1)
        echo	"[nkt_rep_m68k.s] IKBD MIDI DATA SEND DIRECT ENABLED"

        move.w	#0,d1
        move.l 	#_MIDIsendBuffer,a0
        move.w	_MIDIbytesToSend,d1

        cmpi.w   #0,d1
        beq.s   .done       ;if 0 bytes do nothing
.send:
        ;slap data to d0
        move.w	(a0)+,d0	;get word
        ;clr.w	(a0)+ 	;clear it

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
        echo	"[nkt_rep_m68k.s] IKBD MIDI DATA SEND DIRECT DISABLED"
        endif
; prepare next tick
        move.l    update,$114		;slap interrupt
        move.b    tData,$fffffa25.w	;set TiC data
        move.b    tMode,d0
        lsl.b     #4,d0
        ori.b     #$3f,d0
        and.b     d0,$fffffa1d.w       ;set div mode (4,5,6 bits TiC, 0,1,2 TiB)

.finish:
        bset.b    #5,$fffffa09.w       ;enable TiC
        bset.b    #5,$fffffa15.w
        bset.b	  #5,$fffffa11.w  	; clear IRQ in service Bit TiC
        move.w   (sp)+,sr               ; restore Status Register
        movem.l   (sp)+,d0-7/a0-6	; restore registers

        rte


replayNktTB:
        movem.l   d0-7/a0-6,-(a7)	;save registers
        move.w    sr,-(a7)

        or.w        #$0700,sr               ;disable interrupts
        clr.b     $fffffa1b             ; stop TiB
        jsr 	_updateStepNkt      ; update sequence state and send events / copy events to internal send buffer

        if (IKBD_MIDI_SEND_DIRECT==1)
        echo	"[midiReplay.s] IKBD MIDI DATA SEND DIRECT ENABLED"

        move.w	#0,d1
        move.l 	#_MIDIsendBuffer,a0
        move.w	_MIDIbytesToSend,d1

        cmpi.w   #0,d1
        beq.s   .done       ;if 0 bytes do nothing
.send:
      ; slap data to d0
      move.w	(a0)+,d0	;get word
      ;clr.w	(a0)+ 	;clear it

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
        move.b    tData,$fffffa21	;set data
        move.b    tMode,$fffffa1b	;div mode
        bset.b    #0,$fffffa07		;go!
        bset.b    #0,$fffffa13
.finish:
        bclr.b	  #0,$fffffa0f  	;clear IRQ in service bit TiB
        move.w    (a7)+,sr             ;restore sr
        movem.l   (a7)+,d0-7/a0-6	;restore registers
        rte


; installs / deinstalls MIDI replay (single/multitrack) on selected Timer Interrupt

; MFP_TiC     - installs / deinstalls replayRout on TiC
;             note: this non intrusive, steals original TiC vector and adds special jumptable
;             which launches music update routine if needed and after that runs old Timer C handler
; MFP_TiB     - standard custom vector
; MFP_TiA     - * unimplemented *

; #####################################################################################
_NktInstallReplayRout:
        movem.l	d0-d7/a0-a6,-(sp)
        move.l	$40(sp),d1  	; mode         ; MFP interrupt mode
        move.l  $44(sp),d0  	; data         ; MFP interrupt data
        move.w	$48(sp),timerReplayType       ; timer type

        move.w  #0,isMultitrackReplay
        move.w  #0,midiIntCounter

        bsr.w	_super_on
        move.w	sr,-(sp)                ;save status register
        or.w	#$0700,sr               ;turn off all interupts

; TODO check timerReplay type parameter

        ; MFP_TiB
        move.l  #replayNktTB,update
        move.b	d1,tMode                ;save parameters for later
        move.b	d0,tData

        clr.b     $fffffa1b             ;turn off tb

        move.l	  $120,oldVector
        move.l    update,$120		;slap interrupt

        move.b    d0,$fffffa21		;put data
        move.b    d1,$fffffa1b		;put mode
        bset.b    #0,$fffffa07
        bset.b    #0,$fffffa13

; MFP_TiC todo
;       TODO

        move.w 	  (sp)+,sr 		;restore Status Register
        bsr.w	  _super_off
        movem.l (sp)+,d0-d7/a0-a6	;restore registers
        rts

_NktDeinstallReplayRout:
        movem.l	  d0-d7/a0-a6,-(sp)
        bsr.w	_super_on

        move.w	sr,-(a7)		;save status register
        or.w	#$0700,sr
; MFP_TiB
        clr.b     $fffffa1b	;turn off tb
        move.l	 oldVector,$120	;save old tb

; MFP_TiC todo
        move.w	(sp)+,sr	;restore Status Register

        bsr.w	_super_off
        movem.l (sp)+,d0-d7/a0-a6
        rts
