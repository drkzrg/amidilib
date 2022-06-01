#ifndef VARTYPES_H
#define VARTYPES_H

/**  Copyright 2011-21 Pawel Goralski
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "core/assert.h"

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#else
StaticAssert(0,"C99 standard or better is not supported OR not set in C language compiler flags!");
#endif 

typedef uint8_t     uint8;
typedef int8_t      int8;
typedef uint16_t    uint16;
typedef int16_t     int16;
typedef uint32_t    uint32;
typedef int32_t     int32;
typedef float 		fp32;
typedef double 		fp64;
typedef int64_t     int64; 	//non-standard!
typedef uint64_t 	uint64; //non-standard!
typedef uintptr_t   uintptr;
typedef size_t      MemSize;
typedef bool        Bool;

// function pointers
typedef void (*funcPtrVoidVoid)(void);
typedef void (*funcPtrVoidConstUint)(const uint32);

#define PACK_ATTR __attribute__((packed))
#define FAST_CALL __attribute__((fastcall))

#ifdef USE_INLINE

#if (__STDC_VERSION__ >= 199901L) 
#define AM_INLINE inline        /* use standard inline */
#define AM_FORCE_INLINE __attribute__((always_inline))
#else
#define AM_INLINE              /* no inline */
#define AM_FORCE_INLINE
#endif
#else
#define AM_INLINE              /* no inline */
#define AM_FORCE_INLINE
#endif

#define AM_EXTERN extern

#define STRINGISE_IMPL(x) #x
#define STRINGISE(str) STRINGISE_IMPL( str )

#define CONCAT_IMPL(x,y) (x ## y)
#define CONCAT(x1,y2)(CONCAT_IMPL( x1, y2 ))

// checks compiler builtin variable sizes 
void compilerSanityCheck(void);

// check processor endianess ( processor sanity check ;) )
// returns:
// true - for little endian, x86, PS2
// false / for big endian - // Big Endian - GameCube, Atari
const Bool checkEndianess();

#define AM_OK ((int16)(0))
#define AM_ERR ((int16)(-1))

#endif
