
	include 'm68k_defs.inc'

	XDEF _strlen

	TEXT

_strlen:
    ifd   __VASM

    ifne __VASM & m68000

		move.l    4(sp), a0
		moveq     #0,d0
		bra.s     .l1
.l2:    
		addq.l    #1,d0
.l1:    
		tst.b     (a0)+
		bne.s     .l2
		rts
    
    else

; alternative implementation for cpus > m68000

		move.l    4(sp), a0
		moveq     #0,d0
		bra.s     .l1
.l2:    
		addq.l    #1,d0
.l1:    tst.b     (a0)+
		bne.s     .l2
		rts

	endif
	
	else
      ; you're compiling without VASM or VASM version is too old
    endif
    