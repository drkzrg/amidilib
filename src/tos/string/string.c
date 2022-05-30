
#include "vartypes.h"
#include "amstring.h"
#include <limits.h>

int strncmp(const char *s, const char *t, size_t n)
{
    int cc;
    if (n==0) return 0;
    do { cc = (*s++ - *t++); }
        while (!cc && s[-1] && --n>0);
    return cc;
}

void bcopy(const void *_src, void *_dest, size_t _len)
{
    char* src = (char*)_src;
    char* dest = (char*)_dest;

    if (dest < src){
        while (_len--){
            *dest++ = *src++;
        }
    } else {
        char *lasts = src + (_len-1);
        char *lastd = dest + (_len-1);

        while (_len--){
            *(char *)lastd-- = *(char *)lasts--;
        }
    }
}

int stricmp(const char *s, const char *t)
{
    int cc;
    do { cc = tolower(*s++) - tolower(*t++); }
        while (!cc && s[-1]);
    return cc;
}

int strnicmp(const char *s, const char *t, size_t n)
{
    int cc;
    if (n==0) return 0;
    do { cc = tolower(*s++) - tolower(*t++); }
        while (!cc && s[-1] && --n>0);
    return cc;
}

char *strcpy(char *dest, const char* src)
{
    char *ret = dest;
    while (!!((*dest++) = (*src++)));
    return ret;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *ret = dest;

    do {
        if (!n--)
            return ret;
    } while (!!((*dest++) = (*src++)));

    while (n--)
        *dest++ = 0;
    return ret;
}

char *strcat(char *dest, const char *src)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while (!!((*dest++) = (*src++)));
    return ret;
}

void *memchr(const void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char*)s;
    while( n-- )
        if( *p != (unsigned char)c )
            p++;
        else
            return p;
    return 0;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

char *strlwr(char * string)
{
    char * cp;

    for (cp=string; *cp; ++cp) {
        if ('A' <= *cp && *cp <= 'Z') {
            *cp += 'a' - 'A';
        }
    }

    return(string);
}

char *strupr(char* _str)
{
    char *str = _str;

    if (str)
        while (*str){
            *str = toupper(*str);
            ++str;
        }

    return _str;
}

char *strdup(const char *s)
{
    return strcpy((char*) malloc( strlen(s)+1 ), s);
}

void* memmove(void *destination, const void *source, size_t n)
{
    char* dest = (char*)destination;
    char* src = (char*)source;

    /* No need to do that thing. */
    if (dest == src)
        return destination;
    
    /* Check for destructive overlap.  */
    if (src < dest && dest < src + n) {
        /* Destructive overlap ... have to copy backwards.  */
        src += n;
        dest += n;
        while (n-- > 0)
            *--dest = *--src;
    } else {
        /* Do an ascending copy.  */
        while (n-- > 0)
            *dest++ = *src++;
    }

    return destination;
}

char *strrchr(const char *s, int c)
{
    const char *cp = s + strlen(s);

    do
    {
        if (*cp == (char) c) 
        {
            return (char*)cp;  
        }

    } while (--cp >= s);

    return NULL;
}

#include <mint/osbind.h>

void *malloc (size_t size)
{

#if defined(TARGET_ST)
    return (void*)Malloc(size);
#else
    //return (void*)Mxalloc( size, MX_PREFTTRAM);
    //TODO: detect machine
    return (void*)Mxalloc( size, MX_PREFTTRAM);
#endif
}

void * calloc ( size_t num, size_t size )
{
  char *ptr = (char*) Mxalloc( size*num , MX_PREFTTRAM);
   memset( ptr, 0, size*num );
  return ptr;
}

void free(void* ptr)
{
    Mfree ( ptr );
}

void* realloc(void* ptr, size_t size)
{
  char *newptr = (char*) Mxalloc( size , MX_PREFTTRAM);
  memcpy ( newptr, ptr, size );
  Mfree ( ptr );
  return newptr;
}

#define ISSPACE(c) ((c) == ' '||(c) == '\t')
#define ISDIGIT(c) ((c) >= '0' && (c) <= '9')

static inline unsigned long __mul32 (long in, long mul, char *overflow) {
    unsigned long ret;
    __asm__(
    "   clr.l       %0              \n" /* ret = 0                                  */
    "   bra     .start      \n"
    ".loop:                     \n" /* }                                            */
    "   lsr.l       #1, %3      \n" /* mul >>= 1                                */
    "   beq     .end            \n" /* if (mul == 0) goto end;          */

    "   add.l       %2, %2      \n" /* in <<= 1;                                */
    "   bcs     .overfl     \n" /* overflow?    goto overfl;            */
    ".start:                        \n"
    "   btst.l  #0, %3      \n" /* if (mul & 0x01) {                    */
    "   beq     .loop           \n"
    "   add.l       %2, %0      \n" /*   ret = ret + in;                    */
    "   bcc     .loop           \n" /*   if(!overflow) goto loop;       */
    ".overfl:                   \n"
    "   scs     %1              \n"
    ".end:                      \n"
    : "=d"(ret), "+g"(*overflow), "+d"(in) ,"+d"(mul)
    );
    return ret;
}

static inline
unsigned long __add32 (long in, long add, char *overflow) {

    __asm__(
    "   add.l       %2, %0      \n"
    "   bcc     .add32_end  \n"
    "   st          %1              \n"
    ".add32_end:                \n"
    : "+d"(in), "+g"(*overflow)
    : "g"(add)
    );
    return in;
}

unsigned long __strtoul_internal(const char *nptr, char **endptr, int base, int *sign)
{
    long ret = 0L;
    const char *ptr = nptr;
    int val;
    short ret_ok=0;
    char overflow=0;

    if(base!=0 && 2 > base && base > 36)
        goto error;

    while(*ptr && ISSPACE(*ptr)) ptr++; /* skip spaces */

    if( (*sign = (*ptr == '-')) )
        ptr++;

    if(!*ptr)
        goto error;

    if(*ptr == '0') {
        ret_ok = 1;
        switch (*++ptr&~0x20) {
        case 'B':
            if(base != 0 && base != 2) goto error;
            base = 2;
            ptr++;
            break;
        case 'X':
            if(base != 0 && base != 16) goto error;
            base = 16;
            ptr++;
            break;
        default:
            if(base == 0)
                base = 8;
            break;
        }
    } else if(base == 0)
        base = 10;

    for( ;*ptr; ptr++) {
        if(ISDIGIT(*ptr))
            val = *ptr - '0';
        else {
            val = 10 + (*ptr&~0x20 /*TOUPPER*/) - 'A';
            if(val < 10) val = 37;
        }
        ret_ok = 1;
        if(val >= base) break;
        if(!overflow) {
            if(ret)
                ret = __mul32(ret, base, &overflow);
            ret = __add32(ret, val, &overflow);
        }
    }
    if(ret_ok) {
        if(endptr) *endptr = (char*)ptr;
        return overflow ? ULONG_MAX : ret;
    }
error:
    if(endptr) *endptr = (char*)nptr;
    /* TODO set errno */
    return 0LL;
}

long strtol(const char *nptr, char **endptr, int base) 
{
    int sign;
    unsigned long ret = __strtoul_internal(nptr, endptr, base, &sign);
    return ret>LONG_MAX ? (sign ? LONG_MIN : LONG_MAX) : (sign ? -ret : ret);
}

unsigned long strtoul(const char *nptr, char **endptr, int base) 
{
    int sign;
    unsigned long ret = __strtoul_internal(nptr, endptr, base, &sign);
    return sign ? -ret : ret;
}

#include "ctypeint.h"

int tolower(int c)
{
    return isupper(c) ? ((c) + 'a' - 'A') : (c);
}

int toupper(int c)
{
    return islower(c) ? ((c) - 'a' + 'A') : (c);
}
