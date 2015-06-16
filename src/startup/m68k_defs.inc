;m68k cpu defs
		ifd	__VASM		
;this is VASM specific 
m68000 		equ 	$00000001
m68010 		equ 	$00000002
m68020 		equ 	$00000004
m68030 		equ 	$00000008
m68040 		equ 	$00000010
m68060 		equ 	$00000020
m68881 		equ 	$00000040
m68882 		equ 	m68881
m68851 		equ 	$00000080
cpu32 		equ 	$00000100
mcfa 		equ 	$00000200
mcfaplus 	equ 	$00000400
mcfb 		equ 	$00000800
mcfc 		equ 	$00001000
mcfhwdiv 	equ 	$00002000
mcfmac 		equ 	$00004000
mcfemac 	equ 	$00008000
mcfusp 		equ 	$00010000
mcffpu 		equ 	$00020000
mcfmmu 		equ 	$00040000

__mc68000__	equ	(__VASM&$1)
__mc68010__	equ	(__VASM&$2)
__mc68020__	equ	(__VASM&$4)
__mc68030__	equ	(__VASM&$8)
__mc68040__	equ	(__VASM&$10)
__mc68060__	equ	(__VASM&$20)
__mc68882__	equ	(__VASM&$40)    ; mc68881/mc68882
__mc68851__	equ	(__VASM&$80)    ; mmu
__mc68040_60__  equ     (__VASM&$30)
__mc68020_60__  equ     (__VASM&$3c)

		endif


