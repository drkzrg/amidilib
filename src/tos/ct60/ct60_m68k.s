;    Copyright 2007-2021 Pawel Goralski
;    
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.

;Export labels
      XDEF _ct60_ReadCoreTemperature 	; CT60  XBIOS function 0xc60a 
      XDEF _ct60_RWparameter 		; CT60  XBIOS function 0xc60b 
      XDEF _ct60_cache 			; CT60 XBIOS function 0xc60c 
      XDEF _ct60_FlushCache 		; CT60 XBIOS function 0xc60d 
      XDEF _ct60_CacheCtrl 		; CacheCtrl() XBIOS function 160 

;/* ************ Implementation *************** */
                TEXT
                even
_ct60_ReadCoreTemperature:
		move.w	4(sp),-(sp)
		move.w	#$c60a,-(sp)
		trap	#14
		addq.l	#4,sp
		RTS

_ct60_RWparameter:
		move.l	4(sp),-(sp)	
		move.l	8(sp),-(sp)
		move.w	12(sp),-(sp)
		move.w	#$c60b,-(sp)
		trap	#14
		lea.l	12(sp),sp	
		RTS
_ct60_cache:
		move.w	4(sp),-(sp)
		move.w	#$c60c,-(sp)
		trap	#14
		addq.l	#4,sp
		RTS
_ct60_FlushCache:
		move.w	#$c60d,-(sp)
		trap	#14
		addq.l	#2,sp
		RTS
_ct60_CacheCtrl:
		move.w	4(sp),-(sp)   ;param
		move.w	8(sp),-(sp)   ;opcode
		move.w	#160,-(sp)
		trap	#14
		addq.l	#6,sp
		RTS
