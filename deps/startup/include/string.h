
#ifndef STRING_H_
#define STRING_H_

#include <stdlib.h>

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN char *strcat(char *dst, const char *src);
EXTERN char *strcpy(char *dst, const char *src);
EXTERN size_t strlen(const char *s);
EXTERN char *strncat(char *dst, const char *src, size_t max);
EXTERN int strcmp(const char *s1, const char *s2);
EXTERN int stricmp(const char *s1, const char *s2);
EXTERN int strcasecmp(const char *s1, const char *s2);
EXTERN int strncmp(const char *s1, const char *s2, size_t max);
EXTERN int strnicmp(const char *s1, const char *s2, size_t max);
EXTERN int strncasecmp(const char *s1, const char *s2, size_t max);
EXTERN char *strncpy(char *dst, const char *src, size_t max);
EXTERN char *strdup(const char *s1);
EXTERN char *strndup(const char *s1, size_t n);
EXTERN char *strchr(const char *s, int c);
EXTERN char *strstr(const char *s, const char *wanted);
EXTERN size_t strspn(const char *s, const char *accept);
EXTERN size_t strcspn(const char *s, const char *reject);
EXTERN char *strpbrk(const char *s, const char *breakat);
EXTERN char *index(const char *s, int c);
EXTERN char *strrchr(const char *s, int c);
EXTERN char *rindex(const char *s, int c);

EXTERN void bzero(void *s, size_t n);
EXTERN void bcopy(const void *src, void *dst, size_t n);
EXTERN void *memset(void *b, int c, size_t len);
EXTERN void *memcpy(void *dest, const void *src, size_t n);
EXTERN void *memmove(void *dest, const void *src, size_t n);
EXTERN int memcmp(const void *s1, const void *s2, size_t n);
EXTERN void *memchr(const void *s, int c, size_t n);
EXTERN char *strerror(int errnum);
EXTERN char* strerror_r(int errnum, char *buf, size_t bufsize);

EXTERN char *strrev(char *);
EXTERN char *strlwr(char *);

#endif
