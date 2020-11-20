
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef C_VARS_H_
#define C_VARS_H_

#if (defined(DEBUG) || defined(_DEBUG))

#include <assert.h>
#include <stdio.h>

#if __STDC_VERSION__ >= 199901L
#define StaticAssert _StaticAssert
#else
#define StaticAssert 
#endif

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

#define StaticAssert
#   define AssertMsg(condition, message) do { } while (false)
#   define Assert(condition) do { } while (false)
#endif

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
StaticAssert(0,"C99 standard or better is unsupported or not set in c compiler!");
#endif 

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
typedef uintptr_t uintptr;
typedef size_t MemSize;

// function pointers
typedef void (*funcPtrVoidVoid)();
typedef void (*funcPtrVoidConstUint)(const uint32);

typedef enum ERETVAL
{
	E_OK = 0L,
	E_ERR = 1L
} eRetVal;

#define PACK_ATTR __attribute__((packed))
#define FORCE_INLINE inline __attribute__((always_inline))
#define FAST_CALL __attribute__((fastcall))

#ifdef USE_INLINE

#if (__STDC_VERSION__ >= 199901L) 
  #define INLINE inline        /* use standard inline */
#else
  #define INLINE              /* no inline */
#endif
#else
  #define INLINE              /* no inline */
#endif

#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)

// checks compiler builtin variable sizes 
void compilerSanityCheck(void);

// check processor endianess ( processor sanity check ;) )
// returns:
// true - for little endian, x86, PS2
// false / for big endian - // Big Endian - GameCube, Atari
const bool checkEndianess();

#endif
