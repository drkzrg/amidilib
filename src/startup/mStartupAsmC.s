
;custom, minimal startup for C code
;(c) 2011 Mariusz Buras (all), Pawel Goralski (modifications for C)

REDIRECT_OUTPUT_TO_SERIAL	equ	0			;0-output to console,1-output to serial port
		
BASEPAGE_SIZE 	equ 	$100
STACK_SIZE 	equ	$10000		
		
; --------------------------------------------------------------
start:
		move.l	4(sp),a5				;address to basepage
		move.l	$0c(a5),d0				;length of text segment
		add.l	$14(a5),d0				;length of data segment
		add.l	$1c(a5),d0				;length of bss segment
		add.l	#STACK_SIZE+BASEPAGE_SIZE,d0		;length of stackpointer+basepage
		move.l	a5,d1					;address to basepage
		add.l	d0,d1					;end of program
		and.l	#$fffffff0,d1				;align stack
		move.l	d1,sp					;new stackspace

		move.l	d0,-(sp)				;mshrink()
		move.l	a5,-(sp)				;
		clr.w	-(sp)					;
		move.w	#$4a,-(sp)				;
		trap	#1					;
		lea.l	12(sp),sp				;
		

		; clear bss segment
.clearBSS:				
		move.l	$18(a5),a0
		move.l	$1c(a5),d0				;length of bss segment
		
		cmp.l	#0,d0
		beq.s	.skipBSSclear
		moveq	#0,d1
		
.clear:		move.b	d1,(a0)+
		subq.l	#1,d0
		bne.b	.clear
.skipBSSclear:
		
;########################## redirect output to serial		
		if (REDIRECT_OUTPUT_TO_SERIAL==1)  
; redirect to serial
.redirectToSerial:
		move.w #2,-(sp)
		move.w #1,-(sp)
		move.w #$46,-(sp)
		trap #1
		addq.l #6,sp
		endif
	
		jsr	_main

exit:	
		move.w #1,-(sp)
		trap #1
		addq.l #2,sp
		
		clr.w -(sp)
		trap #1
		
		
_basepage:	ds.l	1
_len:		ds.l	1

	
