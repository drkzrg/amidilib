#ifndef STDDEF_H_
#define STDDEF_H_

typedef unsigned long size_t;
typedef signed long ptrdiff_t;

/* Offset of member MEMBER in a struct of type TYPE.  */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#endif
