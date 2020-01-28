#ifndef STDIO_H_
#define STDIO_H_

#include <stdlib.h>
#include <stdarg.h>
#include <osbind.h>

typedef struct __stdio_file FILE;

#define	_IOMAGIC ((int) 0xfedabeeb)	/* Magic number to fill `__magic'.  */

typedef struct __stdio_file {
    int __magic;
    long Handle;      /* GEMDOS handle */
    FILE *__next;     /* Next FILE in the linked list.  */
} FILE;

EXTERN FILE *stdout;
EXTERN FILE *stdin;
EXTERN FILE *stderr;

#define stdaux	(&_StdAuxF)
#define stdprn	(&_StdPrnF)

EXTERN FILE _StdOutF;
EXTERN FILE _StdInF;
EXTERN FILE _StdErrF;
EXTERN FILE _StdAuxF;
EXTERN FILE _StdPrnF;

#define _IONBF	2

#ifndef NULL
#define NULL        ((void *)0)
#endif /* NULL */

#define OPEN_MAX        32
#define FOPEN_MAX       32
#define FILENAME_MAX    128
#define PATH_MAX        128
#define BUFSIZ          1024
#define EOF             (-1)

/*
 * access modes for open()
 */
#define O_RDONLY        0x00    /* open read-only */
#define O_WRONLY        0x01    /* open write-only (this doesn't work apparently on _all_ TOS versions */
#define O_RDWR          0x02    /* open for reading and writing */
#define O_APPEND        0x08    /* position file pointer to end of file before each write */
#define O_CREAT         0x20    /* create file if it doesn't exist */
#define O_TRUNC         0x40    /* truncate to zero size */
#define O_EXCL          0x80    /* ? */

#define SEEK_SET    0   /* Seek from beginning of file.  */
#define SEEK_CUR    1   /* Seek from current position.  */
#define SEEK_END    2   /* Seek from end of file.  */

EXTERN FILE *fopen(const char *path, const char *mode);
EXTERN int fclose(FILE *fp);
EXTERN size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
EXTERN size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
EXTERN int fseek(FILE *fp, long offset, int origin);
EXTERN long ftell(FILE *stream);

EXTERN int fputs(const char *s, FILE *stream);
EXTERN int puts(const char *s);
EXTERN int fputc(int c, FILE *stream);
EXTERN int putc(int c, FILE *stream);

EXTERN int scanf(const char *format, ...);
EXTERN int sscanf(char *, const char *format, ...);
EXTERN int fscanf(FILE *stream, const char *format, ...);
EXTERN int fgetc(FILE *stream);
EXTERN int putchar(int c);
EXTERN int getchar();

EXTERN int fprintf(FILE *stream, const char *format, ...);
EXTERN int vfprintf(FILE *stream, const char *format, va_list ap);

EXTERN int printf(const char *fmt, ...);
EXTERN int snprintf(char *s, size_t size, const char *fmt, ...);
EXTERN int vsnprintf(char *str, size_t size, const char *fmt, va_list va);
EXTERN int sprintf(char *s, const char *format, ...);
EXTERN int vsprintf(char *s, const char *format, va_list va);
EXTERN int puts(const char *s);

EXTERN int open(const char *filename, int access, ...);
EXTERN int close(int fd);
EXTERN int unlink(const char *filename);
EXTERN int printf ( const char * format, ... );

static inline int fileno(FILE *stream) { return stream->Handle; }
static inline int getc(FILE *stream) { return fgetc(stream); }

#endif
