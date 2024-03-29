
	/**  Copyright 2007-2022 Pawel Goralski
    
	This file is part of AMIDILIB.
	See license.txt for licensing information.
	*/

#ifndef __AMSTRING_H__
#define __AMSTRING_H__

#include "vartypes.h"

int memcmp(const void* ptr1, const void* ptr2, size_t num);
void* memcpy(void* destination, const void* source, size_t num);
void* memmove(void* destination, const void* source, size_t num);
void* memset(void* ptr, int value, size_t num);
char* strncpy(char* destination, const char* source, size_t num);
char* strrchr(const char* str, int character);
size_t strlen(const char* str);
int strcmp(const char* str1, const char *str2);
int strncmp(const char* s, const char* t, size_t n);
void* malloc (size_t size);
void* calloc (size_t num, size_t size);
void free (void* ptr);
void* realloc (void* ptr, size_t size);

int tolower(int ch);
int toupper(int ch);

long strtol(const char* nptr, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);


#endif
