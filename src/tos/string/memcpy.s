
	XDEF    _memcpy

    RSRESET
    sp_return:  rs.l    1
    sp_pdst:    rs.l    1
    sp_psrc:    rs.l    1
    sp_size:    rs.l    1

	TEXT
_memcpy:
    move.l        sp_pdst(sp),a0
    move.l        sp_psrc(sp),a1
    move.l        sp_size(sp),d1
    
    lsr.l        #4,d1                  ; num 16-byte blocks total
    move.l        d1,d0
    swap          d0                    ; num 1mb blocks (64k * 16bytes)
    subq.w        #1,d1                 ; num 16-byte blocks remaining
    bcs.s        .ev1mb
.lp1mb:
.lp16b:
    move.l        (a1)+,(a0)+
    move.l        (a1)+,(a0)+
    move.l        (a1)+,(a0)+
    move.l        (a1)+,(a0)+
    dbra           d1,.lp16b

.ev1mb:    
    subq.w        #1,d0
    bpl.s        .lp1mb

    moveq        #16-1,d1
    and.w        sp_size+2(sp),d1
    lsl.b        #4+1,d1
    bcc.s        .n8
    move.l        (a1)+,(a0)+
    move.l        (a1)+,(a0)+
.n8:    
    add.b        d1,d1
    bcc.s        .n4
    move.l        (a1)+,(a0)+
.n4:    
    add.b        d1,d1
    bcc.s        .n2
    move.w        (a1)+,(a0)+
.n2:    
    add.b        d1,d1
    bcc.s        .n1
    move.b        (a1)+,(a0)+
.n1:
    move.l        sp_pdst(sp),d0
    rts
