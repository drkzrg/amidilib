	
	include 'm68k_defs.inc'

	XDEF	_memset

	TEXT

	ifd   __VASM
	ifne __VASM&m68000

_memset:
    move.l        4(sp),a0
    move.l        8(sp),d1
    lsr.l        #4,d1        ; size/16
    move.l        d1,d0
    swap        d0
    subq.w        #1,d1
    bcs.s        .ev1mb
.lp1mb:
.lp16b:    clr.l        (a0)+
    clr.l        (a0)+
    clr.l        (a0)+
    clr.l        (a0)+
    dbra        d1,.lp16b

.ev1mb:
    subq.w        #1,d0
    bpl.s        .lp1mb
    moveq        #16-1,d1
    and.w        8+2(sp),d1    ; s & 15
    lsl.b        #4+1,d1
    bcc.s        .n8
    clr.l        (a0)+        ; 8
    clr.l        (a0)+
.n8:
    add.b        d1,d1
    bcc.s        .n4
    clr.l        (a0)+        ; 4
.n4:
    add.b        d1,d1
    bcc.s        .n2
    clr.w        (a0)+        ; 2
.n2:
    add.b        d1,d1
    bcc.s        .n1
    clr.b        (a0)+        ; 1
.n1:
    ;move.l        4(sp),d0
    rts

    else
; alternative implementation for cpus > m68000

_memset:
    move.l       d2,-(sp)
    move.l       d2,a1
    move.l       4+4(sp),a0
    move.b       4+8+3(sp),d1
    lsl.w        #8,d1
    move.b       4+8+3(sp),d1
    move.w       d1,d2
    swap         d2
    move.w       d1,d2

    move.l       4+12(sp),d1
    lsr.l        #4,d1
    move.l       d1,d0
    swap         d0
    subq.w       #1,d1
    bcs.s        .ev1mb

.lp1mb:
.lp16b:
    move.l       d2,(a0)+
    move.l       d2,(a0)+
    move.l       d2,(a0)+
    move.l       d2,(a0)+
    dbra         d1,.lp16b

.ev1mb:
    subq.w        #1,d0
    bpl.s        .lp1mb

    moveq        #16-1,d1
    and.w        4+12+2(sp),d1
    lsl.b        #4+1,d1
    bcc.s        .n8
    move.l       d2,(a0)+
    move.l       d2,(a0)+
.n8:
    add.b        d1,d1
    bcc.s        .n4
    move.l       d2,(a0)+
.n4:
    add.b        d1,d1
    bcc.s        .n2
    move.w       d2,(a0)+
.n2:
    add.b        d1,d1
    bcc.s        .n1
    move.b       d2,(a0)+
.n1:
    move.l        a1,d2
    move.l        4+4(sp),d0

    move.l        (sp)+,d2
    rts    
    
    endif
    
    else
      ; you're compiling without VASM or VASM version is too old
    endif
