#ifndef MTWIST_H
#define MTWIST_H

/* settings for Atari TOS, PureC compiler */
#undef MT_NO_INLINE			/* Ask for code to be compiled */
#undef __cplusplus
#define MT_NO_INLINE

#define MT_EXTERN 			/* Generate real code for functions */
#define MT_MACHINE_BITS 32

/*
 * $Id: mtwist.h,v 1.17 2007/10/26 07:21:06 geoff Exp $
 *
 * Header file for C/C++ use of the Mersenne-Twist pseudo-RNG.  See
 * http://www.math.keio.ac.jp/~matumoto/emt.html for full information.
 *
 * Author of this header file: Geoffrey H. Kuenning, March 18, 2001.
 *
 * IMPORTANT NOTE: the Makefile must define two machine-specific
 * variables to get optimum features and performance:
 *
 *	MT_NO_INLINE	should be defined if the compiler doesn't support
 *			the "inline" keyword.
 *	MT_NO_LONGLONG	should be defined if the compiler doesn't support a
 *			"long long" type for 64-bit integers
 *	MT_MACHINE_BITS	must be either 32 or 64, reflecting the natural
 *			size of the processor registers.  If undefined, it
 *			will default to a value calculated from limits.h.
 *
 * The first two variables above are defined in an inverted sense
 * because I expect that most compilers will support inline and
 * long-long.  By inverting the sense, this common case will require
 * no special compiler flags.
 *
 * IMPORTANT NOTE: this software requires access to a 32-bit type.  Be
 * sure that "mt_u32bit_t" is set to an unsigned 32-bit integer type.
 * The Mersenne Twist algorithms are not guaranteed to produce correct
 * results with a 64-bit type.
 *
 * The executable part of this software is based on LGPL-ed code by
 * Takuji Nishimura.  The header file is therefore also distributed
 * under the LGPL:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.  You should have
 * received a copy of the GNU Library General Public License along
 * with this library; if not, write to the Free Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * $Log: mtwist.h,v $
 * Revision 1.17  2007/10/26 07:21:06  geoff
 * Introduce, document, and use the new mt_u32bit_t type so that the code
 * will compile and run on 64-bit platforms (although it does not
 * currently use the 64-bit Mersenne Twist algorithm).
 *
 * Revision 1.16  2005/11/11 08:21:39  geoff
 * If possible, try to infer MT_MACHINE_BITS from limits.h.
 *
 * Revision 1.15  2003/09/11 23:56:20  geoff
 * Allow stdio references in C++ files; it turns out that ANSI has
 * blessed it.  Declare the various functions as external even if they're
 * inlined or being compiled directly (in mtwist.c).  Get rid of a #ifdef
 * that can't ever be true.
 *
 * Revision 1.14  2003/09/11 05:50:53  geoff
 * Don't allow stdio references from C++, since they're not guaranteed to
 * work on all compilers.  Disable inlining using the MT_INLINE keyword
 * rather than #defining inline, since doing the latter can affect other
 * files and functions than our own.
 *
 * Revision 1.13  2003/07/01 23:29:29  geoff
 * Refer to streams from the standard library using the correct namespace.
 *
 * Revision 1.12  2002/10/30 07:39:54  geoff
 * Declare the new seeding functions.
 *
 * Revision 1.11  2001/06/19 00:41:16  geoff
 * For consistency with other C++ types, don't put out a newline after
 * the saved data.
 *
 * Revision 1.10  2001/06/18 10:09:24  geoff
 * Fix some places where I forgot to set one of the result values.  Make
 * the C++ state vector protected so the random-distributions package can
 * pass it to the C functions.
 *
 * Revision 1.9  2001/06/18 05:40:12  geoff
 * Prefix the compile options with MT_.
 *
 * Revision 1.8  2001/06/14 10:26:59  geoff
 * Invert the sense of the #define flags so that the default is the
 * normal case (if gcc is normal!).  Also default MT_MACHINE_BITS to 32.
 *
 * Revision 1.7  2001/06/14 10:10:38  geoff
 * Move the critical-path PRNG code into the header file so that it can
 * be inlined.  Add saving/loading of state.  Add functions to seed based
 * on /dev/random or the time.  Add the function-call operator in the C++
 * code.
 *
 * Revision 1.6  2001/06/11 10:00:04  geoff
 * Add declarations of the refresh and /dev/random seeding functions.
 * Change getstate to return a complete state pointer, since knowing the
 * position in the state vector is critical to restoring the state.
 *
 * Revision 1.5  2001/04/23 08:36:03  geoff
 * Remember to zero the state pointer when constructing, since otherwise
 * proper initialization won't happen.
 *
 * Revision 1.4  2001/04/14 01:33:32  geoff
 * Clarify the license
 *
 * Revision 1.3  2001/04/14 01:04:54  geoff
 * Add a C++ class, mt_prng, that makes usage more convenient for C++
 * programmers.
 *
 * Revision 1.2  2001/04/09 08:45:00  geoff
 * Fix the name in the #ifndef wrapper, and clean up some outdated comments.
 *
 * Revision 1.1  2001/04/07 09:43:41  geoff
 * Initial revision
 *
 */

#include <stdio.h>

/*
 * Define an unsigned type that is guaranteed to be 32 bits wide.
 */

typedef unsigned long	mt_u32bit_t;



/*
 * The following value is a fundamental parameter of the algorithm.
 * It was found experimentally using methods described in Matsumoto
 * and Nishimura's paper.  It is exceedingly magic; don't change it.
 */
#define MT_STATE_SIZE	624		/* Size of the MT state vector */

/*
 * Internal state for an MT RNG.  The user can keep multiple mt_state
 * structures around as a way of generating multiple streams of random
 * numbers.
 *
 * In Matsumoto and Nishimura's original paper, the state vector was
 * processed in a forward direction.  I have reversed the state vector
 * in this implementation.  The reason for the reversal is that it
 * allows the critical path to use a test against zero instead of a
 * test against 624 to detect the need to refresh the state.  on most
 * machines, testing against zero is slightly faster.  It also means
 * that a state that has been set to all zeros will be correctly
 * detected as needing initialization; this means that setting a state
 * vector to zero (either with memset or by statically allocating it)
 * will cause the RNG to operate properly.
 */
typedef struct
    {
    mt_u32bit_t		statevec[MT_STATE_SIZE];
					/* Vector holding current state */
    int			stateptr;	/* Next state entry to be used */
    int			initialized;	/* NZ if state was initialized */
    }
    mt_state;



/*
 * Functions for manipulating any generator (given a state pointer).
 */
 void		mts_mark_initialized(mt_state* state);
					/* Mark a PRNG state as initialized */
 void		mts_seed32(mt_state* state, unsigned long seed);
					/* Set random seed for any generator */
 void		mts_seed32new(mt_state* state, unsigned long seed);
					/* Set random seed for any generator */
 void		mts_seedfull(mt_state* state,
			  mt_u32bit_t seeds[MT_STATE_SIZE]);
					/* Set complicated seed for any gen. */
 void		mts_seed(mt_state* state);
					/* Choose seed from random input. */
					/* ..Prefers /dev/urandom; uses time */
					/* ..if /dev/urandom unavailable. */
					/* ..Only gives 32 bits of entropy. */
 void		mts_goodseed(mt_state* state);
					/* Choose seed from more random */
					/* ..input than mts_seed.  Prefers */
					/* ../dev/random; uses time if that */
					/* ..is unavailable.  Only gives 32 */
					/* ..bits of entropy. */
 void		mts_bestseed(mt_state* state);
					/* Choose seed from extremely random */
					/* ..input (can be *very* slow). */
					/* ..Prefers /dev/random and reads */
					/* ..the entire state from there. */
					/* ..If /dev/random is unavailable, */
					/* ..falls back to mt_goodseed().  */
					/* ..Not usually worth the cost.  */
 void		mts_refresh(mt_state* state);
					/* Generate 624 more random values */
 int		mts_savestate(FILE* statefile, mt_state* state);
					/* Save state to a file (ASCII). */
					/* ..Returns NZ if succeeded. */
 int		mts_loadstate(FILE* statefile, mt_state* state);
					/* Load state from a file (ASCII). */
					/* ..Returns NZ if succeeded. */

/*
 * Functions for manipulating the default generator.
 */
 void		mt_seed32(unsigned long seed);
					/* Set random seed for default gen. */
 void		mt_seed32new(unsigned long seed);
					/* Set random seed for default gen. */
 void		mt_seedfull(mt_u32bit_t seeds[MT_STATE_SIZE]);
					/* Set complicated seed for default */
 void		mt_seed(void);	/* Choose seed from random input. */
					/* ..Prefers /dev/urandom; uses time */
					/* ..if /dev/urandom unavailable. */
					/* ..Only gives 32 bits of entropy. */
 void		mt_goodseed(void);
					/* Choose seed from more random */
					/* ..input than mts_seed.  Prefers */
					/* ../dev/random; uses time if that */
					/* ..is unavailable.  Only gives 32 */
					/* ..bits of entropy. */
 void		mt_bestseed(void);
					/* Choose seed from extremely random */
					/* ..input (can be *very* slow). */
					/* ..Prefers /dev/random and reads */
					/* ..the entire state from there. */
					/* ..If /dev/random is unavailable, */
					/* ..falls back to mt_goodseed().  */
					/* ..Not usually worth the cost.  */
 mt_state*	mt_getstate(void);
					/* Get current state of default */
					/* ..generator */
 int		mt_savestate(FILE* statefile);
					/* Save state to a file (ASCII) */
					/* ..Returns NZ if succeeded. */
 int		mt_loadstate(FILE* statefile);
					/* Load state from a file (ASCII) */
					/* ..Returns NZ if succeeded. */



/*
 * Tempering parameters.  These are perhaps the most magic of all the magic
 * values in the algorithm.  The values are again experimentally determined.
 * The values generated by the recurrence relation (constants above) are not
 * equidistributed in 623-space.  For some reason, the tempering process
 * produces that effect.  Don't ask me why.  Read the paper if you can
 * understand the math.  Or just trust these magic numbers.
 */
#define MT_TEMPERING_MASK_B 0x9d2c5680
#define MT_TEMPERING_MASK_C 0xefc60000
#define MT_TEMPERING_SHIFT_U(y) \
			(y >> 11)
#define MT_TEMPERING_SHIFT_S(y) \
			(y << 7)
#define MT_TEMPERING_SHIFT_T(y) \
			(y << 15)
#define MT_TEMPERING_SHIFT_L(y) \
			(y >> 18)

/*
 * Macros to do the tempering.  MT_PRE_TEMPER does all but the last step;
 * it's useful for situations where the final step can be incorporated
 * into a return statement.  MT_FINAL_TEMPER does that final step (not as
 * an assignment).  MT_TEMPER does the entire process.  Note that
 * MT_PRE_TEMPER and MT_TEMPER both modify their arguments.
 */
#define MT_PRE_TEMPER(value)						\
    do									\
	{								\
	value ^= MT_TEMPERING_SHIFT_U(value);				\
	value ^= MT_TEMPERING_SHIFT_S(value) & MT_TEMPERING_MASK_B;	\
	value ^= MT_TEMPERING_SHIFT_T(value) & MT_TEMPERING_MASK_C;	\
	}								\
	while (0)
#define MT_FINAL_TEMPER(value) ((value) ^ MT_TEMPERING_SHIFT_L(value))
#define MT_TEMPER(value)						\
    do									\
	{								\
	value ^= MT_TEMPERING_SHIFT_U(value);				\
	value ^= MT_TEMPERING_SHIFT_S(value) & MT_TEMPERING_MASK_B;	\
	value ^= MT_TEMPERING_SHIFT_T(value) & MT_TEMPERING_MASK_C;	\
	value ^= MT_TEMPERING_SHIFT_L(value);				\
	}								\
	while (0)



/*
 * Functions for generating random numbers.  The actual code of the
 * functions is given in this file so that it can be declared inline.
 * For compilers that don't have the inline feature, mtwist.c will
 * incorporate this file with some clever #defining so that the code
 * actually gets compiled.  In that case, however, "extern"
 * definitions will be needed here, so we give them.
 */


 unsigned long	mts_lrand(mt_state* state);
					/* Generate 32-bit value, any gen. */

 double		mts_drand(mt_state* state);
					/* Generate floating value, any gen. */
					/* Fast, with only 32-bit precision */
 double		mts_ldrand(mt_state* state);
					/* Generate floating value, any gen. */
					/* Slower, with 64-bit precision */

 unsigned long	mt_lrand(void);	/* Generate 32-bit random value */

 double		mt_drand(void);
					/* Generate floating value */
					/* Fast, with only 32-bit precision */
 double		mt_ldrand(void);
					/* Generate floating value */
					/* Slower, with 64-bit precision */




#endif /* MTWIST_H */
