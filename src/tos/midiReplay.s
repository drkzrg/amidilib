
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;    midi sequence replay from custom structure

    include "timing/mfp_m68k.inc"
    include "common_m68k.inc"
    
    xdef _replaySingleTB   ; custom sequence replay handler on timerB
    xdef _replaySingleTC   ; custom sequence replay handler on timerC
    xdef _replayMultiTB    ; custom sequence replay handler on timerB (multi track)
    xdef _replayMultiTC    ; custom sequence replay handler on timerC (multi track)

    xref _updateStepSingle ; our sequence update routine (single track)
    xref _updateStepMulti  ; our sequence update routine (multi track)

    xref update		     ; our installed interrupt routine

	TEXT
; ############## single track
_replaySingleTB:
        or.w    #$2700,sr               ;disable interrupts
	movem.l   d0-7/a0-6,-(a7)	;save registers

        clr.b     $fffffa1b             ; stop TiB
        jsr 	_updateStepSingle	; update sequence state and send events / copy events to internal send buffer

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
	; prepare next tick
        move.l    update,$120		;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
        bset.b    #0,$fffffa07		;go!
        bset.b    #0,$fffffa13
.finish:	
	movem.l   (a7)+,d0-7/a0-6	;restore registers
        bclr.b	  #0,$fffffa0f  	;clear IRQ in service bit
        move.w    #$2300,sr             ;enable interrupts
        rte


_replaySingleTC:
        or.w    #$2700,sr
        movem.l   d0-7/a0-6,-(a7)	;save registers

;       jump to the old interrupt routine afterwards
;       send midi status byte once per 1ms
;       assume 200Hz update for TC (?)

        andi.b  #$0F,$fffffa1d.w        ; disable TC
        jsr 	_updateStepSingle	; update sequence state and send events / copy events to internal send buffer

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
        move.l    update,$114		;slap interrupt
        move.b    _tbData,$fffffa25.w	;set TiC data
        move.b    _tbMode,d0
        lsl.b     #4,d0
        ori.b     #$3f,d0
        and.b     d0,$fffffa1d.w       ;set div mode (4,5,6 bits TiC, 0,1,2 TiB)

        bset.b    #5,$fffffa09.w       ;enable TiC
        bset.b    #5,$fffffa15.w
.finish:
        movem.l   (a7)+,d0-7/a0-6	;restore registers
        bclr.b	  #0,$fffffa0f  	;clear IRQ in service Bit
        move.w    #$2300,sr
        rte

; ################ multi track
_replayMultiTB:
        or.w    #$2700,sr               ;disable interrupts
        movem.l   d0-7/a0-6,-(a7)	;save registers

        clr.b     $fffffa1b             ; stop TiB
        jsr 	_updateStepMulti	; update sequence state and send events / copy events to internal send buffer

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
        ; prepare next tick
        move.l    update,$120		;slap interrupt
        move.b    _tbData,$fffffa21	;set data
        move.b    _tbMode,$fffffa1b	;div mode
        bset.b    #0,$fffffa07		;go!
        bset.b    #0,$fffffa13
.finish:
        movem.l   (a7)+,d0-7/a0-6	;restore registers
        bclr.b	  #0,$fffffa0f  	;clear IRQ in service bit
        move.w    #$2300,sr             ;enable interrupts
        rte


_replayMultiTC:
        or.w    #$2700,sr
        movem.l   d0-7/a0-6,-(a7)	;save registers

;       jump to the old interrupt routine afterwards
;       send midi status byte once per 1ms
;       assume 200Hz update for TC (?)

        andi.b  #$0F,$fffffa1d.w        ; disable TC
        jsr 	_updateStepMulti	; update sequence state and send events / copy events to internal send buffer

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
        move.l    update,$114		;slap interrupt
        move.b    _tbData,$fffffa25.w	;set TiC data
        move.b    _tbMode,d0
        lsl.b     #4,d0
        ori.b     #$3f,d0
        and.b     d0,$fffffa1d.w       ;set div mode (4,5,6 bits TiC, 0,1,2 TiB)

        bset.b    #5,$fffffa09.w       ;enable TiC
        bset.b    #5,$fffffa15.w
.finish:
        movem.l   (a7)+,d0-7/a0-6	;restore registers
        bclr.b	  #0,$fffffa0f  	;clear IRQ in service Bit
        move.w    #$2300,sr
        rte


