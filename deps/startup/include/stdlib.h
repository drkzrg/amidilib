
#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

typedef unsigned long size_t;

EXTERN int atoi(const char *c);
EXTERN long atol(const char *c);
EXTERN void *malloc(size_t size);
EXTERN void *calloc(size_t nmemb, size_t size);
EXTERN void *realloc(void *ptr, size_t size);
EXTERN void free(void *ptr);

EXTERN char *ltoa(long value, char *buffer, int radix);
#define _ltoa(a, b, c) ltoa((a), (b), (c))

EXTERN char *getenv(const char *name);

int atexit(void (*func)(void));
EXTERN void exit(int status);
EXTERN void abort(void);

#ifdef __MSHORT__
#define	RAND_MAX (0x7FFF)
#else
#define	RAND_MAX (0x7FFFFFFFL)
#endif

EXTERN void qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));

EXTERN int rand(void);
EXTERN long lrand(void);
EXTERN void srand48(unsigned int seed);

EXTERN long strtol(const char*, char**, int);
EXTERN unsigned long strtoul(const char*, char**, int);
EXTERN long long strtoll(const char*, char**, int);
EXTERN unsigned long long strtoull(const char*, char**, int);

#define abs(j)  ((j) > 0 ? j : -j)

#define NULL	((void *) 0)

#endif
