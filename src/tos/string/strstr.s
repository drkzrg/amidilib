
	XDEF	_strstr	

	TEXT
_strstr:
	; this code is 52 bytes bigger than the c implementation. But it is way faster.
	;from http://cristi.indefero.net/p/uClibc-cristi/source/tree/ff250619f58caa6b10c951911c43fbb8a34fda8d/libc/string/strstr.c
	movem.l 4(sp),a0-a1
	movem.l d2/a2-a3,-(sp)
	;a0: haystack
	;a1: needle (>=1 char string)
	
	move.b  (a1),d1             ; d1: neddle0
	beq.s   .ret1
	subq.l  #1,a0
.do1:       
	addq.l  #1,a0
	move.b  (a0),d2             ; d2: haystack0
	beq.s   .ret0
	cmp.b   d2,d1
	bne.s   .do1
	
	addq.l  #1,a1
	move.b  (a1)+,d2            ;d2: needle1
	beq.s   .ret1
	bra.s   .jin
	
.for:
.do2:       
	addq.l  #1,a0
	move.b  (a0),d0             ;d0: haystack1
	beq.s   .ret0
	cmp.b   d0,d1
	beq.s   .do2break
	addq.l  #1,a0
	move.b  (a0),d0
	beq.s   .ret0
.shloop:    
	cmp.b   d0,d1
	bne.s   .do2
	
.do2break:
.jin:       
	addq.l  #1,a0
	move.b  (a0),d0
	beq.s   .ret0
	cmp.b   d0,d2
	bne.s   .shloop
	
	move.l  a0,a2               ;a2: rhaystack
	addq.l  #1,a2
	subq.l  #1,a0
	move.l  a1,a3               ;a3: rneedle
	move.b  (a3),d0
	
	cmp.b   (a2),d0
	bne.s   .endif
.do3        
	tst.b   d0
	beq.s   .ret1
	addq.l  #1,a1
	move.b  (a1),d0
	addq.l  #1,a2
	cmp.b   (a2),d0
	bne.s   .do3break
	tst.b   d0
	beq.s   .ret1
	addq.l  #1,a1
	move.b  (a1),d0
	addq.l  #1,a2
	cmp.b   (a2),d0
	beq.s   .do3
.do3break:
.endif:      
	move.l  a3,a1
	tst.b   d0
	bne.s   .for
	
.ret1:       
	move.l   a0,d0
	bra.s   .ret
.ret0:       
	moveq   #0,d0
.ret:        
	movem.l (sp)+,d2/a2-a3
	rts
