
INEXACT_RESULT 		equ $0001
UNDERFLOW 		equ $0002
OVERFLOW 		equ $0004
DIVIDE_BY_ZERO 		equ $0008
INVALID_OPERATION 	equ $0010

; The allowed rounding modes are:
UNKNOWN           equ  -1
ROUND_TO_NEAREST  equ  0 ; round result to nearest representable value
ROUND_TO_ZERO     equ  1 ; round result towards zero
ROUND_TO_PLUS     equ  2 ; round result towards plus infinity
ROUND_TO_MINUS    equ  3 ; round result towards minus infinity

; The allowed values of format are:
NIL          equ  0
SINGLE_FLOAT equ  1
DOUBLE_FLOAT equ  2
LONG_FLOAT   equ  3

; The allowed values for the last operation are:
NOOP         equ  0
ADD          equ  1
MULTIPLY     equ  2
DIVIDE       equ  3
NEGATE       equ  4
COMPARE      equ  5
EXTENDSFDF   equ  6
TRUNCDFSF    equ  7

GREATER equ  1
LESS    equ -1
EQUAL   equ  0

QUIET_NaN   equ $ffffffff
D_MAX_EXP equ $07ff
D_BIAS equ 1022
DBL_MAX_EXP equ D_MAX_EXP - D_BIAS
DBL_MIN_EXP equ 1 - D_BIAS
DBL_MANT_DIG equ 53

; fpCCR offsets
EBITS:      rs.w	1
TRAPE:      rs.w        1
STICK:      rs.w	0
ROUND:      rs.w	ROUND_TO_NEAREST
FORMT:      rs.w	NIL
LASTO:      rs.w        NOOP
OPER1:      rs.l        2
OPER2:      rs.l        2
        rsreset

; enabled float / double support
L_floatex   equ   1
L_double    equ   1

L_ltdf2     equ   1
L_umodsi3   equ   1
L_udivsi3   equ   1
L_mulsi3    equ   1
L_eqdf2     equ   1
L_gedf2     equ   1
