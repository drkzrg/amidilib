
	XDEF	_strcmp

_strcmp:
	move.l    4(sp),a0
	move.l    8(sp),a1
.next:  
	move.b    (a1)+,d0
	beq.s     .end
	cmp.b     (a0)+,d0
	beq.s     .next
	bcs.s     .gtr
	moveq     #-1,d0
	rts
.gtr:   
	moveq     #1,d0
	rts

.end:   
	tst.b     (a0)
	bne.s     .gtr
	moveq     #0,d0
	rts
