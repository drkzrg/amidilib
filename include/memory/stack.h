#ifndef __STACK_H__
#define __STACK_H__

// (c)Pawel Goralski 12'2011
// e-mail: nokturnal@nokturnal.pl

#include "c_vars.h"

//stack for storing game states
#define DEFAULT_MAXSTACK 20

typedef struct {
  tMEMSIZE top;            //top of stack
  tMEMSIZE size;           //current max size of a stack, if we try to go past this threshold, then
                           //it's size will be increased by DEFAULT_MAXSTACK elements
  U32 elementSize;
  void *stack;
} tStack;

//if initialMaxSize==0, then maximal initial size is set to DEFAULT_MAXSTACK
S32 initStack(tStack *pPtr, tMEMSIZE initialMaxSize, U32 elementSize);

//void element has to be of the constant size
void pushStack(tStack *pPtr, void *newElement);
void popStack(tStack *pPtr);
void *getTopStackElement();
BOOL isStackFull(tStack *pPtr);
BOOL isStackEmpty(tStack *pPtr);
void deinitStack(tStack *pPtr);



#endif