#ifndef ASSERT_H
#define ASSERT_H

#ifndef NDEBUG

#include <stdlib.h>
#include <stdio.h>

#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg);

#define ASSERT(expr)                                                         \
  ((void)((expr) ||                                                          \
          (fprintf(stderr, "\r\nAssertion failed: %s, file %s, line %d\r\n", \
                   #expr, __FILE__, __LINE__),                               \
           ((int (*)(void))abort)())))
#else
#define assert(expr)
#endif /* NDEBUG */

#endif
