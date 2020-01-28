
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef C_VARS_H_
#define C_VARS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

// function pointers
typedef void (*funcPtrVoidVoid)();
typedef void (*funcPtrVoidConstUint)(const uint32);

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

// TODO: make it more cross-compiler friendly
// atm it's gcc specific
#define PACK_ATTR __attribute__((packed))
#define FORCE_INLINE inline __attribute__((always_inline))
#define FAST_CALL __attribute__((fastcall))

#ifdef USE_INLINE
  #define INLINE inline        /* use standard inline */
#else
  #define INLINE              /* no inline */
#endif

#ifndef NDEBUG
#include <assert.h>
#include <stdlib.h>

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg);

#define ASSERT(expr)                                                         \
  ((void)((expr) ||                                                          \
          (fprintf(stderr, "\r\nAssertion failed: %s, file %s, line %d\r\n", \
                   #expr, __FILE__, __LINE__),                               \
           ((int (*)(void))abort)())))
#else
#define assert(cond) 
#define STATIC_ASSERT(cond, msg) 
#define ASSERT(expr) (void)

#endif

// checks compiler builtin variable sizes 
void compilerSanityCheck(void);

// check processor endianess ( processor sanity check ;) )
// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari
const bool checkEndianess();

#endif
