
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "containers/list.h"
#include "timing/mfp.h"
#include "core/logger.h"
#include "core/amprintf.h"

/* adds event to linked list, list has to be inintialised with null */
// event list, temp event

#ifdef EVENT_LINEAR_BUFFER
int16 addEvent(sSequence_t *pSequence, sEventList **listPtr, const sEventBlock_t *eventBlockPtr )
#else
int16 addEvent(sEventList **listPtr, const sEventBlock_t *eventBlockPtr )
#endif
{
 sEventList *pTempPtr = NULL;
 sEventList *pNewItem = NULL;

if(*listPtr!=NULL)
{
  /* list not empty, start at very first element */
  /* and iterate till the end */
  
  pTempPtr=*listPtr;
			
  while((pTempPtr->pNext != NULL))
  {
    pTempPtr=pTempPtr->pNext;
  }
    
  /* insert at the end of list */
#ifdef EVENT_LINEAR_BUFFER
  if(copyEvent(pSequence, eventBlockPtr, &pNewItem) == AM_OK)
#else
  if(copyEvent(eventBlockPtr, &pNewItem) == AM_OK)
#endif
  {
      pNewItem->pNext=NULL;		/* next node is NULL for new node */
      pNewItem->pPrev=pTempPtr;	/* prev node is current element node */

      /* add newly created list node to our list */
      pTempPtr->pNext=pNewItem;
      return AM_OK;  
  }	
}
else
{
  
#ifdef EVENT_LINEAR_BUFFER
  if(copyEvent(pSequence, eventBlockPtr, listPtr) == AM_OK)
#else
  if(copyEvent(eventBlockPtr, listPtr) == AM_OK)
#endif
  {
      (*listPtr)->pPrev=NULL;		/* first element in the list, no previous item */
      (*listPtr)->pNext=NULL;
      return AM_OK;
  }
 }
 
 return AM_ERR;
}

#ifdef EVENT_LINEAR_BUFFER
int16 copyEvent(sSequence_t *pSequence, const sEventBlock_t *src, sEventList **dest)
#else
int16 copyEvent(const sEventBlock_t *src, sEventList **dest)
#endif
{
  #ifdef DEBUG_MEM
    amTrace("copyEvent() src: %p dst: %p"NL,src,dest);
  #endif
    
#ifdef EVENT_LINEAR_BUFFER
    (*dest) = linearBufferAlloc(&(pSequence->eventBuffer), sizeof(sEventList));
#else
    (*dest)=(sEventList *)gUserMemAllocCb(sizeof(sEventList),PREFER_TT,0);
#endif
    
    if((*dest)==NULL)
    {
		  amTrace("copyEvent() out of memory [event block]"NL,0);
		#ifdef EVENT_LINEAR_BUFFER
      linearBufferPrintInfo(&(pSequence->eventBuffer));
		#endif
	   return AM_ERR;
    }
    else
    {
		  (*dest)->eventBlock.uiDeltaTime=src->uiDeltaTime;
		  (*dest)->eventBlock.type = src->type;
      (*dest)->eventBlock.eventCb.size = src->eventCb.size;
		  (*dest)->eventBlock.eventCb.func = src->eventCb.func;
      (*dest)->eventBlock.dataPtr=NULL;
		
		/* allocate memory for event data and copy them to the new destination */
#ifdef EVENT_LINEAR_BUFFER
        (*dest)->eventBlock.dataPtr = linearBufferAlloc(&(pSequence->eventBuffer),(src->eventCb.size * sizeof(uint8)));
#else
        (*dest)->eventBlock.dataPtr = gUserMemAllocCb((src->eventCb.size * sizeof(uint8)),PREFER_TT,0);
#endif
    
	   if((*dest)->eventBlock.dataPtr==NULL)
     {
	    amTrace("copyEvent() out of memory [callback block]"NL,0);
	    
#ifdef EVENT_LINEAR_BUFFER
      linearBufferPrintInfo(&(pSequence->eventBuffer));
#endif
		return AM_ERR;
	   }
     else
     {
       amMemCpy((*dest)->eventBlock.dataPtr,src->dataPtr,(src->eventCb.size * sizeof(uint8)));
       return AM_OK;
	   }
   }
  return AM_ERR;
}
#ifdef EVENT_LINEAR_BUFFER
uint32 destroyList(sSequence_t *pSequence,sEventList **listPtr)
#else
uint32 destroyList(sEventList **listPtr)
#endif
{
sEventList *pTemp=NULL,*pCurrentPtr=NULL;

#ifdef DEBUG_MEM
AM_PRINT((const uint8 *)"destroyList()"NL);
#endif
	
	if(*listPtr!=NULL){
	  
#ifdef EVENT_LINEAR_BUFFER
      linearBufferFree(&(pSequence->eventBuffer));
	  *listPtr=0; //that's right :P
#else	
	  /*go to the end of the list */
	  pTemp=*listPtr;
			
	  while(pTemp->pNext!=NULL){
	    pTemp=pTemp->pNext;
	  }
			
	  /* we are at the end of list, rewind to the previous element */
	  pCurrentPtr=pTemp->pPrev;
			
	  /* iterate to the begining */
	  while(pCurrentPtr!=NULL){

	  if(((pCurrentPtr->eventBlock.dataPtr)>(void *)(0L))){
	    
	    //free up additionally memory for specific blocks
	    switch(pCurrentPtr->eventBlock.type)
      {
	      case T_META_MARKER:
        {
		      sMarker_EventBlock_t *pTemp=(sMarker_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
          gUserMemFree(pTemp->pMarkerName,0);
	      }break;
	      
	      case T_META_CUEPOINT:
        {
		      sCuePoint_EventBlock_t *pTemp=(sCuePoint_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
          gUserMemFree(pTemp->pCuePointName,0);
	      }break;

	      case T_SYSEX:
        {
		      sSysEX_EventBlock_t *pTemp=(sSysEX_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
          gUserMemFree(pTemp->pBuffer,0);
	      }break;
	    };
	    //release event block itself
        gUserMemFree(pCurrentPtr->eventBlock.dataPtr,0);
	  }

    gUserMemFreeCb(pCurrentPtr->pNext,0);
	  pCurrentPtr=pCurrentPtr->pPrev;
	}
	/* we are at first element */
	/* remove it */
    gUserMemFree(*listPtr,0);
	
    #endif
  }
return 0;
}

void printEventList(const sEventList *listPtr)
{
	sEventList *pTemp=NULL;	
		
	if(listPtr!=NULL)
  {
		/* iterate through list */
		pTemp=(sEventList *)listPtr;
		uint32 counter=0;
	
		while(pTemp!=NULL)
    {
		/* print */
		  const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
		  printEventBlock(pBlock);		
		  counter++;
		  pTemp=pTemp->pNext;
		}
	}
}

/* prints out events data  */
void printEventBlock(const sEventBlockPtr_t pPtr)
{
   uint8 *pbuf=NULL;
   int x=0;

   amTrace("*********** event info: "NL,0);
   amTrace("delta: %lu\t",pPtr->uiDeltaTime);
   amTrace("event type: %d\t",pPtr->type);
   amTrace("copy/send event callback: %p\t",pPtr->eventCb.func);
   amTrace("copy/send data size: %u\t",pPtr->eventCb.size);
   amTrace("data pointer: %p"NL,pPtr->dataPtr);
   amTrace("data: \t",0);
   
   pbuf = (uint8 *)pPtr->dataPtr;
   
   for(x=0;x<pPtr->eventCb.size;x++)
   {
    amTrace("0x%x ",pbuf[x]);
   }

   amTrace(""NL,0);
    switch((uint16)(pPtr->type))
    {
    
    case T_NOTEON:
    {
      amTrace("T_NOTEON: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_NOTEOFF:{
      amTrace("T_NOTEOFF: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_NOTEAFT:{
      amTrace("T_NOTEAFT: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_CONTROL:
    {
      amTrace("T_CONTROL: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_PRG_CH:
    {
      amTrace("T_PRG_CH: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_CHAN_AFT:
    {
        amTrace("T_CHAN_AFT: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_PITCH_BEND:
    {
      amTrace("T_PITCH_BEND: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_META_SET_TEMPO:
    {
      amTrace("T_META_SET_TEMPO: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_META_EOT:
    {
      amTrace("T_META_EOT: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_META_CUEPOINT:
    {
      amTrace("T_META_CUEPOINT: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_META_MARKER:
    {
      amTrace("T_META_MARKER: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_META_SET_SIGNATURE:
    {
      amTrace("T_META_SET_SIGNATURE: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    case T_SYSEX:
    {
      amTrace("T_SYSEX: block pointer: %p, function pointer: %p"NL,pPtr,pPtr->eventCb.func);
    } break;
    default:
    {
      AssertMsg(false,"printEventBlock() error unknown/unsupported event type."NL);
      amTrace("printEventBlock() error unknown/unsupported event type %d"NL,(uint16)(pPtr->type));
    } break;
 }

  /* decode stored event */
}

