
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef C_VARS_H_
#define C_VARS_H_

/* ###################################################################################
#  INCLUDES
################################################################################### */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef DEBUG_BUILD
#include <assert.h>
#endif

/* ###################################################################################
#  TYPEDEFS
################################################################################### */

typedef uint8_t     U8;
typedef int8_t      S8;
typedef uint16_t    U16;
typedef int16_t     S16;
typedef uint32_t    U32;
typedef int32_t     S32;
typedef	float 		F32;
typedef float 		FP32;
typedef double 		FP64;
typedef bool        BOOL;
typedef int64_t     S64; //non-standard!
typedef unsigned long long 	U64; //non-standard!

typedef size_t tMEMSIZE;
typedef void (*VOIDFUNCPTR)();

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

void compilerSanityCheck(void);

// check processor endianess ( processor sanity check ;) )

// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari
const BOOL checkEndianess();

#ifdef USE_INLINE
  #define INLINE inline        /* use standard inline */
#else
  #define INLINE              /* no inline */
#endif

#define STATIC_ASSERT(cond,str) _Static_assert (cond, str);

#endif
