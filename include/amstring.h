
	/**  Copyright 2007-2022 Pawel Goralski
    
	This file is part of AMIDILIB.
	See license.txt for licensing information.
	*/

#ifndef __AMSTRING_H__
#define __AMSTRING_H__

#include "vartypes.h"

AM_EXTERN int memcmp(const void* ptr1, const void* ptr2, size_t num);
AM_EXTERN void* memcpy(void* destination, const void* source, size_t num);
AM_EXTERN void* memmove(void* destination, const void* source, size_t num);
AM_EXTERN void* memset(void* ptr, int value, size_t num);
AM_EXTERN char* strncpy(char* destination, const char* source, size_t num);
AM_EXTERN char* strrchr(const char* str, int character);
AM_EXTERN size_t strlen(const char* str);
AM_EXTERN int strcmp(const char* str1, const char *str2);
AM_EXTERN int strncmp(const char* s, const char* t, size_t n);
AM_EXTERN void* malloc (size_t size);
AM_EXTERN void* calloc (size_t num, size_t size);
AM_EXTERN void free (void* ptr);
AM_EXTERN void* realloc (void* ptr, size_t size);

AM_EXTERN int tolower(int ch);
AM_EXTERN int toupper(int ch);

AM_EXTERN long strtol(const char* nptr, char** endptr, int base);
AM_EXTERN unsigned long strtoul(const char* nptr, char** endptr, int base);


#endif
