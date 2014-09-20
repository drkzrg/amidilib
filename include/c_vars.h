
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __C_VARS_H__
#define __C_VARS_H__

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include	<stdio.h>

#ifdef DEBUG_BUILD
#include <assert.h>
#endif

/* ###################################################################################
#  TYPEDEFS
################################################################################### */

typedef unsigned char 		U8;
typedef signed char 		S8;
typedef unsigned short int 	U16;
typedef signed short int 	S16;
typedef unsigned long U32;
typedef signed long 		S32;
typedef	float 			F32;
typedef float 			FP32;
typedef double 			FP64;
typedef unsigned short int  BOOL;
typedef signed long long	S64; //non-standard!
typedef unsigned long long 	U64; //non-standard!

typedef size_t tMEMSIZE;
typedef void (*VOIDFUNCPTR)();

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

void compilerSanityCheck(void);

// check processor endianess ( processor sanity check ;) )

// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari
BOOL checkEndianess();

#ifdef USE_INLINE
  #define INLINE inline        /* use standard inline */
#else
  #define INLINE              /* no inline */
#endif

#endif
