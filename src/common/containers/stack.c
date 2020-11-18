
#include "containers/stack.h"
#include "memory/memory.h"

static const uint32 DEFAULT_MAXSTACK = 20;

int32 initStack(sStack *stackState, const uint32 elementSize, const uint32 initialStackSize)
{
  // allocate memory
  const MemSize memAllocSize = elementSize * initialStackSize;
  void *pNewStack = (void *)amMallocEx(memAllocSize, PREFER_TT);

  if(pNewStack == 0L)
  {
    return 1;
  } 
  
  amMemSet(pNewStack,0,memAllocSize);
  
  stackState->top = 0UL;  
  stackState->stack = pNewStack;
  stackState->elementSize = elementSize;
  stackState->size = initialStackSize;

  return 0;  
}

// void element has to be of the constant size
void pushStack(sStack *stackState, void *newElement)
{

  if(stackState->top == stackState->size)
  {
      //stack underflow
      stackState->size = stackState->size + DEFAULT_MAXSTACK;
      
      if(amRealloc(stackState->stack,stackState->size * stackState->elementSize) == NULL)
      {
        // Houston we have a problem. nothing can be done...
        puts("Warning: Stack overflow!\r\t");
      }
    return;
  }
  else
  {
    const uintptr dst=((uintptr)stackState->stack)+((++stackState->top)*(stackState->elementSize));
    amMemCpy((void *)dst,newElement,stackState->elementSize);
    return;
  }
}

void *getTopStackElement(sStack *stackState)
{
  //we assume stack is not empty
    uintptr adr = ((uintptr)stackState->stack) + (stackState->top * stackState->elementSize);

   //return removed element 
    return (void *)adr;
}

void popStack(sStack *stackState)
{

  if(stackState->top==0)
  { 
    // stack underflow
    puts("Warning: Stack underflow!\r\t");
  }
  else 
  {
    --stackState->top;
  }
}

bool isStackFull(const sStack *stackState)
{
  if(stackState->top == (stackState->size-1)) 
  {
    return true;
  }

  return false;
}

bool isStackEmpty(const sStack *stackState)
{

  if(stackState->top == 0) 
  {
    return true;
  }
  
  return false;
}

void deinitStack(sStack *pPtr)
{
  amFree(pPtr->stack);
}

