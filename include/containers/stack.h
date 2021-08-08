#ifndef _STACK_H_
#define _STACK_H_

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"

typedef struct SSTACK 
{
  MemSize top;          // top of stack
  MemSize size;         // current max size of a stack, if we try to go past this threshold, then
						// it's size will be reallocated and increased by DEFAULT_MAXSTACK elements
  uint32 elementSize;
  void *stack;
} sStack;

int32 initStack(sStack *stackState,  const uint32 elementSize, const uint32 initialMaxSize);
void pushStack(sStack *stackState, void *newElement); // void element has to be of the constant size
void popStack(sStack *stackState);
void* getTopStackElement(sStack *stackState);
Bool isStackFull(const sStack *stackState);
Bool isStackEmpty(const sStack *stackState);
void deinitStack(sStack *stackState);

#endif
