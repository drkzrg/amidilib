
//C startup
#include <mint/osbind.h>
#include <mint/falcon.h>

#include <string.h>
#include <stdio.h>

#include <limits.h>
#include <ctype.h>
#include <stdlib.h>


void __main(){}

// ----------------------------------------------------------------------------------------

int puts( const char* pText ){
	Cconws ( pText );
	Cconws ( "\r\n" );	
	return 0;
}

// ----------------------------------------------------------------------------------------

void* memset ( void * ptr, int value, size_t num )
{
	unsigned char* pTemp = (unsigned char*) ptr;
	
	while ( num-- )
	{
		*pTemp++=value;
	}
	
	return ptr;
}


void * malloc ( size_t size )
{
	//printf ("malloc %d\r\n",size);
	//return (void*)Mxalloc( size, MX_PREFTTRAM);
	//TODO: detect machine
	return (void*)Mxalloc( size, MX_PREFTTRAM);
}

// ----------------------------------------------------------------------------------------

void * calloc ( size_t num, size_t size )
{
	char *ptr = (char*) Mxalloc( size*num , MX_PREFTTRAM);
	memset( ptr, 0, size*num );
	return ptr;
}

// ----------------------------------------------------------------------------------------

void free ( void * ptr )
{
	if ( ptr )
	{
		Mfree ( ptr );
	}
}

// ----------------------------------------------------------------------------------------

void * realloc ( void * ptr, size_t size )
{
	//printf ("realloc %d\r\n",size);
	char *newptr = (char*) Mxalloc( size , MX_PREFTTRAM);
	memcpy ( newptr, ptr, size );
	Mfree ( ptr );
	return newptr;
}

// ----------------------------------------------------------------------------------------

size_t strlen ( const char * str )
{
	size_t len = 0;
	while ( *(char*)str ++ != 0 ) len++;
	return len;
}

// ----------------------------------------------------------------------------------------

int strcmp(const char *s1, const char *s2)
{
    while((*s1 && *s2) && (*s1++ == *s2++));
    return *(--s1) - *(--s2);
}

// ----------------------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------------------

int atoi( const char* pStr ) 
{ 
  int iRetVal = 0; 
  int iTens = 1;
 
  if ( pStr )
  {
    const char* pCur = pStr; 
    while (*pCur) 
      pCur++;
 
    pCur--;
 
    while ( pCur >= pStr && *pCur <= '9' && *pCur >= '0' ) 
    { 
      iRetVal += ((*pCur - '0') * iTens);
      pCur--; 
      iTens *= 10; 
    }
  } 
  return iRetVal; 
} 

// ----------------------------------------------------------------------------------------

