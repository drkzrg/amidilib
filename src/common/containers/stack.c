
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "containers/stack.h"
#include "memory/memory.h"

// if initialMaxSize==0, then maximal initial size is set to DEFAULT_MAXSTACK
int32 initStack(tStack *pPtr, MemSize initialMaxSize, uint32 elementSize){
  pPtr->top=0;  
  pPtr->stack=0;
  pPtr->elementSize=elementSize;
  
  if(initialMaxSize==0) {
    pPtr->size = DEFAULT_MAXSTACK;
  }else{
    pPtr->size = initialMaxSize;
  }
  
  //allocate memory
  void *pNewStack=0;
  
  pNewStack=(void *)amMallocEx(elementSize*pPtr->size,PREFER_TT);

  if(pNewStack==0) return 1;
    amMemSet(pNewStack,0,elementSize*pPtr->size);
    pPtr->stack=pNewStack;
  
  return 0;  
}


//void element has to be of the constant size
void pushStack(tStack *pPtr, void *newElement){
  if(pPtr->top == pPtr->size){
      //stack underflow
      pPtr->size=pPtr->size + DEFAULT_MAXSTACK;
      
      if(amRealloc(pPtr->stack,pPtr->size*pPtr->elementSize)==NULL){
        //Houston we have a problem. nothing can be done...
        puts("Warning: Stack overflow!\r\t");
      }
    return;
  }else{
    uint32 dst=((uint32)pPtr->stack)+((++pPtr->top)*(pPtr->elementSize));
    amMemCpy((void *)dst,newElement,pPtr->elementSize);
    return;
  }
}

void *getTopStackElement(tStack *pPtr){
  
  //we assume stack is not empty
    uint32 adr=((uint32)pPtr->stack)+(pPtr->top*pPtr->elementSize);

   //return removed element 
    return (void *)adr;
}

void popStack(tStack *pPtr){

  if(pPtr->top==0){ 
    //stack underflow
    puts("Warning: Stack underflow!\r\t");
    
  }else {
    --pPtr->top;
  }
}

const bool isStackFull(const tStack *pPtr){
  if(pPtr->top==(pPtr->size-1)) 
    return TRUE;
  else 
    return FALSE;
}

const bool isStackEmpty(const tStack *pPtr){

  if(pPtr->top==0) 
    return TRUE;
  else 
    return FALSE;
  
}

void deinitStack(tStack *pPtr){
  
  amFree(pPtr->stack);
  
  pPtr->top=0;  
  pPtr->elementSize=0;
  pPtr->size = 0;
}

