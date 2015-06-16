; custom, minimal startup for C code
; (c) 2011 Mariusz Buras (all), Pawel Goralski (modifications for C)

		include 'm68k_defs.inc'
		xdef	_memcpy
		xdef	_memchr
		xdef	_memcmp
		

		;for m68k target only
		ifd   __VASM		;check if VASM
		ifne __VASM&m68000		

		;add conditional compilation of this symbols for bare mc68000
		xdef ___mulsi3
		xdef ___udivsi3
		xdef ___umodsi3
		xdef ___clzsi2
		xdef ___ctzsi2
		
		endif
		else
		  ; you're compiling without VASM or VASM version is too old
		endif		

; --------------------------------------------------------------
_memcpy:	
	move.l	4(sp),a0
	move.l	8(sp),a1
	move.l	12(sp),d1

	lsr.l	#4,d1
	move.l	d1,d0
	swap.w	d0
	subq.w	#1,d1	
	bmi.b	.1
.2:
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	dbf		d1,.2
.1:
	subq.w	#1,d0
	bpl.b	.2

	move.l	12(sp),d1
	and.w	#$f,d1
	subq.w	#1,d1
	bmi.b	.3
.4:
	move.b	(a1)+,(a0)+
	dbf	d1,.4
.3:
	move.l	4(sp),d0
	rts

_memchr:
;TODO
      rts

_memcmp:
;TODO
      rts
	
      ifd   __VASM
      ifne __VASM&m68000
      
___mulsi3:	
      move.w   6(sp),d0
      move.l   d0, a0        	; save B
      mulu.w   8(sp), d0    	; d0 holds B * C
      move.w   10(sp), d1
      move.l   d1, a1        	; save D
      mulu.w   4(sp), d1    	; %d1 holds A * D
      add.w    d1, d0        	; %d0 holds (B * C) + (A * D)
      swap    d0
      clr.w   d0             	; %d0 holds ((B * C) + (A * D)) << 16
      exg     a0, d0        	; restore B
      move.l  a1, d1        	; restore D
      mulu.w  d1, d0        	; %d0 holds B * D
      add.l   a0, d0        	; final result
      rts
	
___udivsi3:
        move.l   d2, -(sp)
        move.l   12(sp), d1     ; d1 = divisor 
        move.l   8(sp), d0      ; d0 = dividend 
  
        cmp.l   #$10000, d1 	; divisor >= 2 ^ 16 ?   
        bcc.s   .L3              ; then try next algorithm 
        move.l  d0, d2
        clr.w   d2
        swap    d2
        divu    d1, d2          ; high quotient in lower word 
        move.w  d2, d0          ; save high quotient 
        swap    d0
        move.w  10(sp), d2      ; get low dividend + high rest 
        divu    d1, d2          ; low quotient 
        move.w  d2, d0
        bra.s   .L6		;jra     L6
 
.L3:    move.l  d1, d2          ; use d2 as divisor backup 
.L4:    lsr.l   #1, d1     	; shift divisor 
        lsr.l   #1, d0     	; shift dividend 
        cmp.l   #$10000, d1 	; still divisor >= 2 ^ 16 ?  
        bcc.s	.L4
        divu    d1, d0          ; now we have 16 bit divisor 
        and.l   #$ffff, d0 	; mask out divisor, ignore remainder 
 
 ; Multiply the 16 bit tentative quotient with the 32 bit divisor.  Because of
 ; the operand ranges, this might give a 33 bit product.  If this product is
 ;greater than the dividend, the tentative quotient was too large. 
         move.l  d2,d1
         mulu  d0,d1         	; low part, 32 bits 
         swap   d2
         mulu  d0, d2         	; high part, at most 17 bits 
         swap    d2             ; align high part with low part 
         tst.w   d2             ; high part 17 bits? 
         bne.s     .L5          ; if 17 bits, quotient was too large 
         add.l   d2, d1         ; add parts 
         bcs.s   .L5            ; if sum is 33 bits, quotient was too large 
         cmp.l   8(sp), d1      ; compare the sum with the dividend 
         bls.s   .L6            ; if sum > dividend, quotient was too large 
.L5:	 subq.l   #1, d0        ; adjust quotient 
 
.L6:	move.l   (sp)+, d2
      rts

___umodsi3:
	  ;this requires that __udivsi3 preserve the %a0
	  ; register, and that it returns the modulus in %d1:
            move.l   (sp)+, a0     	; pop the return address
            jsr     ___udivsi3
            move.l   d1, d0        ; move the modulus into %d0
            jmp     (a0)            ; return
	rts

___clzsi2:
;TODO
	rts
___ctzsi2:
;TODO
	rts

	endif
	else
	  ; you're compiling without VASM or VASM version is too old
	endif



