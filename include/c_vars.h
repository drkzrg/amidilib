
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef C_VARS_H_
#define C_VARS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "assert.h"

typedef uint8_t     uint8;
typedef int8_t      int8;
typedef uint16_t    uint16;
typedef int16_t     int16;
typedef uint32_t    uint32;
typedef int32_t     int32;
typedef float 		fp32;
typedef double 		fp64;
typedef int64_t     int64; 			//non-standard!
typedef unsigned long long 	uint64; //non-standard!

typedef size_t MemSize;
typedef void (*VOIDFUNCPTR)();

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

// checks
void compilerSanityCheck(void);

// check processor endianess ( processor sanity check ;) )
// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari
const bool checkEndianess();

#ifdef USE_INLINE
  #define INLINE inline        /* use standard inline */
#else
  #define INLINE              /* no inline */
#endif


#endif
