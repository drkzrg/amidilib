
	XDEF    _memcpy

_memcpy:
	move.l  d2,-(sp)
	move.l  4+4(sp),a0      ;a0=source
	move.l  4+8(sp),a1      ;a1=dest
	move.l  4+12(sp),d2     ;d2=length
	
	cmp.l   #16,d2
	blo.b   .inf16
	
	move.l  a0,d0
	move.l  a1,d1
	and.b   #1,d0
	and.b   #1,d1
	cmp.b   d0,d1
	bne.b   .oddcp
	
	move.l  a0,d0
	tst.b   d1
	beq.b   .alig

;both addr are odd
	move.b  (a1)+,(a0)+
	subq.l  #1,d2
	.alig:
	moveq   #3,d1
	and.l   d2,d1
	sub.l   d1,d2
;=barrier
.c:
	move.l  (a1)+,(a0)+
	subq.l  #4,d2
	bne.b   .c
	move.w  d1,d2
	bra.b   .remai
	
.oddcp: 
	cmp.l   #$10000,d2
	blo.b   .inf16
	move.l  a0,d0
.c2:
	move.b  (a1)+,(a0)+
	subq.l  #1,d2
	bne.b   .c2
	bra.b   .end
	
.inf16:
	move.l  a0,d0
.remai:
	subq.w  #1,d2
	blo.b   .end
.c3:
	move.b  (a1)+,(a0)+
	dbf     d2,.c3
	
.end:
	move.l  (sp)+,d2
	
	rts
