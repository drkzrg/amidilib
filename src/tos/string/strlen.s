
		XDEF _strlen

_strlen:
		move.l    4(sp), a0
		moveq     #0,d0
		bra.s     .l1
.l2:    
		addq.l    #1,d0
.l1:    tst.b     (a0)+
		bne.s     .l2
		rts
