#ifndef NASSERT_H
#define NASSERT_H

/**  Copyright 2011-21 Pawel Goralski
    This file is part of Nidhoggr engine.
    See license.txt for licensing information.
*/

#if defined(FINAL)

// disable all asserts in final build
#define StaticAssert(condition, message)
#define AssertMsg(condition, message) 
#define Assert(condition) 

#else

#include <assert.h>

#if __STDC_VERSION__ >= 199901L
#define StaticAssert(condition, message) _Static_assert(condition, message)
#else
// no static asserts in versions below c99
#define StaticAssert(condition, message) 
#endif

#endif //FINAL

#endif
