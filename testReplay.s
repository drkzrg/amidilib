;    Copyright 2007-2010 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

    xdef _installReplayRout
    xdef _deinstallReplayRout
    xref _playNote
    xref super_on
    xref super_off
    xref _oldTB
    xref _counter
    xref _tbMode
    xref _tbData

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

_installReplayRout:
	move.l	a0,_currentSeqPtr
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
	clr.b     $fffffa1b
	eor.w	  #$0f0,$ffff8240

;<----- insert your code here ----- >
	move.l	_currentSeqPtr,a0
	move.l	currentTempo(a0),d0 ;get current tempo, current slot in the sequence,
	move.l	currentIdx(a0),d1   ;ptr to current sequence
	move.l	seqPtr(a0),a1       
	move.l	state(a0),d2	    ;current state of sequence

        cmp.l	#0,d2		    ;is sequence stopped?
	bne.s	.paused		    ;not equal check if we have pause
        
	;we have stop so we have to reset all counters and set
	;current track to 0 index
	move.l	#0,currentTempo(a0) 
	move.l	#0,currentIdx(a0)   
	move.l	#0,_counter
	;we go straight to exit, nothing to do
	bra.s  .exit

.paused:
	cmp.l	#4,d2		    ;is sequence paused?
	bne.s	.increaseCounter
	
	bra.s  .exit

.increaseCounter:
;======================================
        move.l	_counter,d0
	addq.l	#1,d0
	move.l	d0,_counter
;======================================
.exit:
;<----- insert your code here ----- >
        move.l    #update,$120	;slap interrupt 
	move.b    _tbData,$fffffa21	;set data
	move.b    _tbMode,$fffffa1b	;div mode
	bset.b    #0,$fffffa07		;go!
	bset.b    #0,$fffffa13
.finish:	
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
	
        move.w	(sp)+,sr		;restore Status Register

	bsr.w	super_off
	movem.l (sp)+,d0-d7/a0-a6
	rts


	bss
	even
_currentSeqPtr:		ds.l	1

	RSRESET ;
currentTempo	rs.l	1
currentIdx	rs.l	1
seqPtr		rs.l	1
state		rs.l	1


