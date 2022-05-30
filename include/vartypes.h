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
#define INLINE inline        /* use standard inline */
#define FORCE_INLINE __attribute__((always_inline))
#else
#define INLINE              /* no inline */
#define FORCE_INLINE
#endif
#else
#define INLINE              /* no inline */
#define FORCE_INLINE
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

#if (defined(DEBUG) || defined(_DEBUG))

#include "core/libc.h"

#   define AssertMsg(condition, message) \
    do { \
        if (! (condition)) { \
            printf("Assertion %s failed in %s at %d line. Message: %s\n", \
            #condition, __FILE__, __LINE__, message);\
        } \
    } while (false)

#   define Assert(condition) \
    do { \
        if (! (condition)) { \
            printf("Assertion %s failed in %s at %d line. \n", \
            #condition, __FILE__, __LINE__);\
        } \
    } while (false)

#else

#define AssertMsg(condition, message) do { } while (false)
#define Assert(condition) do { } while (false)
#endif

#define AM_OK 0
#define AM_ERR -1

#endif
