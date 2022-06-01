
#include "containers/stack.h"
#include "memory/memory.h"

static const uint32 DEFAULT_MAXSTACK = 20;

int32 initStack(sStack *stackState, const uint32 elementSize, const uint32 initialStackSize)
{
  // allocate memory
  const MemSize memAllocSize = elementSize * initialStackSize;
  void *pNewStack = gUserMemAlloc(memAllocSize, MF_PREFER_FASTRAM,0);

  if(pNewStack)
  {
    amMemSet(pNewStack,0,memAllocSize);
  
    stackState->top = 0UL;  
    stackState->stack = pNewStack;
    stackState->elementSize = elementSize;
    stackState->size = initialStackSize;
    return 1;
  } 

  return 0;  
}

// void element has to be of the constant size
void pushStack(sStack *stackState, void *newElement)
{
  if(stackState->top == stackState->size)
  {
      //stack underflow
      stackState->size = stackState->size + DEFAULT_MAXSTACK;
      
      if(gUserMemRealloc(stackState->stack,stackState->size * stackState->elementSize,0) == NULL)
      {
        AssertMsg(0,"Out of memory. Couldn't reallocate stack...\r\t");
      }
  }
  else
  {
    const uintptr dst=((uintptr)stackState->stack)+((++stackState->top)*(stackState->elementSize));
    amMemCpy((void *)dst,newElement,stackState->elementSize);
  }
}

void *getTopStackElement(sStack *stackState)
{
  // we assume stack is not empty
  AssertMsg(stackState->top != 0,"Stack is empty!");

  uintptr adr = ((uintptr)stackState->stack) + (stackState->top * stackState->elementSize);

  // return removed element 
  return (void *)adr;
}

void popStack(sStack *stackState)
{
  AssertMsg(stackState->top!=0,"Warning: Stack underflow!\r\t");
  --stackState->top;
}

Bool isStackFull(const sStack *stackState)
{
  if(stackState->top == (stackState->size-1)) 
  {
    return TRUE;
  }

  return FALSE;
}

Bool isStackEmpty(const sStack *stackState)
{
  if(stackState->top == 0) 
  {
    return TRUE;
  }
  
  return FALSE;
}

void deinitStack(sStack *pPtr)
{
  AssertMsg(pPtr!=NULL,"Invalid stack!\r\t");
  gUserMemFree(pPtr->stack,0);
}
