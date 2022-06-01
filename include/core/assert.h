#ifndef NASSERT_H
#define NASSERT_H

/**  Copyright 2022 Pawel Goralski

    This file is part of Amidilib.
    See license.txt for licensing information.
*/

#if (defined(DEBUG) || defined(_DEBUG))
#include "core/logger.h"

#define AssertFatal(condition,message) \
{\
    bool isFatal = false;\
    do {\
        if (!(condition))\
        {\
            logOutputTrace(TL_ERR_FATAL, __FILE__, __LINE__, "condition: (%s %s)", #condition, message);\
            isFatal = true;\
        }\
    } while (isFatal);\
}

#define AssertMsg(condition,message) \
do {\
    if (!(condition))\
    {\
        logOutputTrace(TL_ERR_ERROR, __FILE__,__LINE__, "condition: (%s %s)", #condition, message);\
    }\
   } while (0)
    
#define Assert(condition) \
    do \
    { \
        if (! (condition))\
        { \
            logOutputTrace(TL_ERR_ERROR, __FILE__,__LINE__,#condition);\
        } \
    } while (0)

#define AssertMsgRts(condition,message,rval) \
do {\
    if (!(condition))\
    {\
        logOutputTrace(TL_ERR_ERROR, __FILE__,__LINE__, "condition: (%s %s) in %", #condition, message, __FUNCTION__);\
        return rval;\
    }\
   } while (0)


#elif defined(NDEBUG)
// disable all asserts in final build

#if __STDC_VERSION__ >= 199901L
#define StaticAssert(condition, message) _Static_assert(condition, message)
#else
// no static asserts in versions below c99
#define StaticAssert(condition, message) ((void)0)
#endif

#define AssertFatal(condition,message) ((void)0)
#define AssertMsg(condition, message) ((void)0)
#define AssertMsgRts(condition,message,rval) if(!condition) return rval
#define Assert(condition) ((void)0)

#else

#if __STDC_VERSION__ >= 199901L
#define StaticAssert(condition, message) _Static_assert(condition, message)
#else
// no static asserts in versions below c99
#define StaticAssert(condition, message) ((void)0)
#endif

#define AssertFatal(condition,message) ((void)0)
#define AssertMsg(condition, message) ((void)0)
#define AssertMsgRts(condition,message,rval) if(!condition) return rval
#define Assert(condition) ((void)0)

#endif


#endif
