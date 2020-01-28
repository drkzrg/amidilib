#ifndef ARRAYS_H
#define ARRAYS_H

/* buf.h --- growable memory buffers for C99
 * This is free and unencumbered software released into the public domain.
 *
 *   arr_size(v)     : return the number of elements in the buffer (size_t)
 *   arr_capacity(v) : return the total capacity of the buffer (size_t)
 *   arr_free(v)     : destroy and free the buffer
 *   arr_push(v, e)  : append an element E to the end
 *   arr_pop(v)      : remove and return an element E from the end
 *   arr_grow(v, n)  : increase buffer capactity by (ptrdiff_t) N elements
 *   arr_trunc(v, n) : set buffer capactity to exactly (ptrdiff_t) N elements
 *   arr_clear(v, n) : set buffer size to 0 (for push/pop)
 *
 * Note: arr_push(), arr_grow(), arr_trunc(), and arr_free() may change
 * the buffer pointer, and any previously-taken pointers should be
 * considered invalidated.
 *
 * Example usage:
 *
 *     float *values = 0;
 *     for (size_t i = 0; i < 25; i++)
 *         arr_push(values, rand() / (float)RAND_MAX);
 *     for (size_t i = 0; i < arr_size(values); i++)
 *         printf("values[%zu] = %f\n", i, values[i]);
 *     arr_free(values);
 */

#include <stddef.h>
#include <stdlib.h>

#ifndef BUF_INIT_CAPACITY
#  define BUF_INIT_CAPACITY 8
#endif

#ifndef BUF_ABORT
#  define BUF_ABORT abort()
#endif

struct buf {
    size_t capacity;
    size_t size;
    char buffer[];
};

#define arr_ptr(v) \
    ((struct buf *)((char *)(v) - offsetof(struct buf, buffer)))

#define arr_free(v) \
    do { \
        if (v) { \
            free(arr_ptr((v))); \
            (v) = 0; \
        } \
    } while (0)

#define arr_size(v) \
    ((v) ? arr_ptr((v))->size : 0)

#define arr_capacity(v) \
    ((v) ? arr_ptr((v))->capacity : 0)

#define arr_push(v, e) \
    do { \
        if (arr_capacity((v)) == arr_size((v))) { \
            (v) = arr_grow1(v, sizeof(*(v)), \
                            !arr_capacity((v)) ? \
                              BUF_INIT_CAPACITY : \
                              arr_capacity((v))); \
        } \
        (v)[arr_ptr((v))->size++] = (e); \
    } while (0)

#define arr_pop(v) \
    ((v)[--arr_ptr(v)->size])

#define arr_grow(v, n) \
    ((v) = arr_grow1((v), sizeof(*(v)), n))

#define arr_trunc(v, n) \
    ((v) = arr_grow1((v), sizeof(*(v)), n - arr_capacity(v)))

#define arr_clear(v) \
    ((v) ? (arr_ptr((v))->size = 0) : 0)


static void *
arr_grow1(void *v, size_t esize, ptrdiff_t n)
{
    struct buf *p;
    size_t max = (size_t)-1 - sizeof(struct buf);
    if (v) {
        p = arr_ptr(v);
        if (n > 0 && p->capacity + n > max / esize)
            goto fail; /* overflow */
        p = realloc(p, sizeof(struct buf) + esize * (p->capacity + n));
        if (!p)
            goto fail;
        p->capacity += n;
        if (p->size > p->capacity)
            p->size = p->capacity;
    } else {
        if ((size_t)n > max / esize)
            goto fail; /* overflow */
        p = malloc(sizeof(struct buf) + esize * n);
        if (!p)
            goto fail;
        p->capacity = n;
        p->size = 0;
    }
    return p->buffer;
fail:
    BUF_ABORT;
    return 0;
}

#endif
