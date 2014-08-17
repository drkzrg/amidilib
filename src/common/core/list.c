
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
    
#include <assert.h>
#include <string.h>

#include "amidilib.h"
#include "amlog.h"
#include "list/list.h"

#include "timing/mfp.h"

/* adds event to linked list, list has to be inintialised with null */
// event list, temp event

#ifdef EVENT_LINEAR_BUFFER
S16 addEvent(sSequence_t *pSequence, sEventList **listPtr, sEventBlock_t *eventBlockPtr )
#else
S16 addEvent(sEventList **listPtr, sEventBlock_t *eventBlockPtr )
#endif
{
 sEventList *pTempPtr=NULL;
 sEventList *pNewItem=NULL;

if(*listPtr!=NULL){
  /* list not empty, start at very first element */
  /* and iterate till the end */
  
  pTempPtr=*listPtr;
			
  while((pTempPtr->pNext != NULL)){
    pTempPtr=pTempPtr->pNext;
  }
    
  /* insert at the end of list */
#ifdef EVENT_LINEAR_BUFFER
  if(copyEvent(pSequence, eventBlockPtr, &pNewItem)>=0){
#else
  if(copyEvent(eventBlockPtr, &pNewItem)>=0){
#endif

      pNewItem->pNext=NULL;		/* next node is NULL for new node */
      pNewItem->pPrev=pTempPtr;	/* prev node is current element node */

      /* add newly created list node to our list */
      pTempPtr->pNext=pNewItem;
      return 1;  
  }	
  
}else{
  
#ifdef EVENT_LINEAR_BUFFER
  if(copyEvent(pSequence, eventBlockPtr, listPtr)>=0){
#else
  if(copyEvent(eventBlockPtr, listPtr)>=0){
#endif

      (*listPtr)->pPrev=NULL;		/* first element in the list, no previous item */
      (*listPtr)->pNext=NULL;
      return 1;
  }
  
 }
 
 return -1;
}

#ifdef EVENT_LINEAR_BUFFER
S16 copyEvent(sSequence_t *pSequence, const sEventBlock_t *src, sEventList **dest){
#else
S16 copyEvent(const sEventBlock_t *src, sEventList **dest){
#endif

  #ifdef DEBUG_MEM
    amTrace((const U8 *)"copyEvent() src: %p dst: %p\n",src,dest);
  #endif
    
#ifdef EVENT_LINEAR_BUFFER
    (*dest)=linearBufferAlloc(&(pSequence->eventBuffer), sizeof(sEventList));
#else
    (*dest)=(sEventList *)amMallocEx(sizeof(sEventList),PREFER_TT);
#endif
    
    if((*dest)==NULL){
		amTrace((const U8 *)"copyEvent() out of memory [event block]\n");
		#ifdef EVENT_LINEAR_BUFFER
            linearBufferPrintInfo(&(pSequence->eventBuffer));
		#endif
	return -1;
    }else{
		(*dest)->eventBlock.uiDeltaTime=src->uiDeltaTime;
		(*dest)->eventBlock.type = src->type;

#ifdef IKBD_MIDI_SEND_DIRECT
        (*dest)->eventBlock.copyEventCb.size = src->copyEventCb.size;
		(*dest)->eventBlock.copyEventCb.func=src->copyEventCb.func;
#else
        (*dest)->eventBlock.sendEventCb.size = src->sendEventCb.size;
        (*dest)->eventBlock.sendEventCb.func=src->sendEventCb.func;
#endif
        (*dest)->eventBlock.dataPtr=NULL;
		
		/* allocate memory for event data and copy them to the new destination */

#ifdef IKBD_MIDI_SEND_DIRECT
#ifdef EVENT_LINEAR_BUFFER
        (*dest)->eventBlock.dataPtr = linearBufferAlloc(&(pSequence->eventBuffer),(src->copyEventCb.size * sizeof(U8)));
#else
        (*dest)->eventBlock.dataPtr = amMallocEx((src->copyEventCb.size * sizeof(U8)),PREFER_TT);
#endif
#else
#ifdef EVENT_LINEAR_BUFFER
        (*dest)->eventBlock.dataPtr = linearBufferAlloc(&(pSequence->eventBuffer),(src->sendEventCb.size * sizeof(U8)));
#else
        (*dest)->eventBlock.dataPtr = amMallocEx((src->sendEventCb.size * sizeof(U8)),PREFER_TT);
#endif

#endif

    
	if((*dest)->eventBlock.dataPtr==NULL){
	    amTrace((const U8 *)"copyEvent() out of memory [callback block]\n");
	    
		#ifdef EVENT_LINEAR_BUFFER
            linearBufferPrintInfo(&(pSequence->eventBuffer));
		#endif

		return -1;
	}else{
#ifdef IKBD_MIDI_SEND_DIRECT
        amMemCpy((*dest)->eventBlock.dataPtr,src->dataPtr,(src->copyEventCb.size * sizeof(U8)));
#else
        amMemCpy((*dest)->eventBlock.dataPtr,src->dataPtr,(src->sendEventCb.size * sizeof(U8)));
#endif
	    return 1;
	}
   }
   return -1;
}
#ifdef EVENT_LINEAR_BUFFER
U32 destroyList(sSequence_t *pSequence,sEventList **listPtr){
#else
U32 destroyList(sEventList **listPtr){
#endif

sEventList *pTemp=NULL,*pCurrentPtr=NULL;

#ifdef DEBUG_MEM
amTrace((const U8 *)"destroyList()\n");
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
	    switch(pCurrentPtr->eventBlock.type){

	      case T_META_MARKER:{
		sMarker_EventBlock_t *pTemp=(sMarker_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
        amFree(pTemp->pMarkerName);
	      }break;
	      
	      case T_META_CUEPOINT:{
		sCuePoint_EventBlock_t *pTemp=(sCuePoint_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
        amFree(pTemp->pCuePointName);
	      }break;

	      case T_SYSEX:{
		sSysEX_EventBlock_t *pTemp=(sSysEX_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
        amFree(pTemp->pBuffer);
	      }break;
	    };
	    //release event block itself
        amFree(pCurrentPtr->eventBlock.dataPtr);
	  }

      amFree(pCurrentPtr->pNext);
	  pCurrentPtr=pCurrentPtr->pPrev;
	}
	/* we are at first element */
	/* remove it */
    amFree(listPtr);
	
    #endif
  }
return 0;
}

void printEventList(const sEventList *listPtr){
	sEventList *pTemp=NULL;	
		
	if(listPtr!=NULL){
		/* iterate through list */
		pTemp=(sEventList *)listPtr;
		U32 counter=0;
	
		while(pTemp!=NULL){
		/* print */
		  const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
		  printEventBlock(pBlock);		
		  counter++;
		  pTemp=pTemp->pNext;
		}
	}
}

/* prints out events data  */
void printEventBlock(const sEventBlockPtr_t pPtr){
 
   U8 *pbuf=NULL;
   int x=0;

   amTrace((const U8*)"*********** event info: \n");
   amTrace((const U8*)"delta: %lu\t",pPtr->uiDeltaTime);
   amTrace((const U8*)"event type: %d\t",pPtr->type);

#ifdef IKBD_MIDI_SEND_DIRECT
   amTrace((const U8*)"copy event callback: %p\t",pPtr->copyEventCb.func);
   amTrace((const U8*)"copy data size: %lu\t",pPtr->copyEventCb.size);
#else
   amTrace((const U8*)"send event callback: %p\t",pPtr->sendEventCb.func);
   amTrace((const U8*)"send data size: %lu\t",pPtr->sendEventCb.size);
#endif

   amTrace((const U8*)"data pointer: %p\n",pPtr->dataPtr);
   amTrace((const U8*)"data: \t");
   
   pbuf=(U8 *)pPtr->dataPtr;
   
#ifdef IKBD_MIDI_SEND_DIRECT
   for(x=0;x<pPtr->copyEventCb.size;x++){
    amTrace((const U8*)"0x%x ",pbuf[x]);
   }

   amTrace("\n");
    switch((U16)(pPtr->type)){
    case T_NOTEON:{
      amTrace((const U8*)"T_NOTEON: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
      return;
     }
     break;

      case T_NOTEOFF:{
      amTrace((const U8*)"T_NOTEOFF: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
      return;
    }
       break;

      case T_NOTEAFT:{
      amTrace((const U8*)"T_NOTEAFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
      return;
      }
      break;

      case T_CONTROL:{
      amTrace((const U8*)"T_CONTROL: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
      return;
      }
      break;

    case T_PRG_CH:{
        amTrace((const U8*)"T_PRG_CH: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
        return;
    }
      break;
    case T_CHAN_AFT:{
        amTrace((const U8*)"T_CHAN_AFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
        return;
    }
        break;

    case T_PITCH_BEND:{
          amTrace((const U8*)"T_PITCH_BEND: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
          return;
    }
      break;
    case T_META_SET_TEMPO:{
          amTrace((const U8*)"T_META_SET_TEMPO: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
          return;
    }
    break;

    case T_META_EOT:{
     amTrace((const U8*)"T_META_EOT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
     return;
    }break;

    case T_META_CUEPOINT:{
     amTrace((const U8*)"T_META_CUEPOINT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
     return;
    }break;

    case T_META_MARKER:{
     amTrace((const U8*)"T_META_MARKER: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
     return;
    }break;

    case T_META_SET_SIGNATURE:{
     amTrace((const U8*)"T_META_SET_SIGNATURE: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
     return;
    }break;

    case T_SYSEX:{
     amTrace((const U8*)"T_SYSEX: block pointer: %p, function pointer: %p\n",pPtr,pPtr->copyEventCb.func);
    return;

    }break;

    default:{
          amTrace((const U8*)"printEventBlock() error unknown/unsupported event type %d\n",(U16)(pPtr->type));
          return;
    }
    break;
 }

#else
   for(x=0;x<pPtr->sendEventCb.size;x++){
    amTrace((const U8*)"0x%x ",pbuf[x]);
   }

   amTrace("\n");
    switch((U16)(pPtr->type)){
    case T_NOTEON:{
      amTrace((const U8*)"T_NOTEON: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
      return;
     }
     break;

      case T_NOTEOFF:{
      amTrace((const U8*)"T_NOTEOFF: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
      return;
    }
       break;

      case T_NOTEAFT:{
      amTrace((const U8*)"T_NOTEAFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
      return;
      }
      break;

      case T_CONTROL:{
      amTrace((const U8*)"T_CONTROL: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
      return;
      }
      break;

    case T_PRG_CH:{
        amTrace((const U8*)"T_PRG_CH: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
        return;
    }
      break;
    case T_CHAN_AFT:{
        amTrace((const U8*)"T_CHAN_AFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
        return;
    }
        break;

    case T_PITCH_BEND:{
          amTrace((const U8*)"T_PITCH_BEND: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
          return;
    }
      break;
    case T_META_SET_TEMPO:{
          amTrace((const U8*)"T_META_SET_TEMPO: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
          return;
    }
    break;

    case T_META_EOT:{
     amTrace((const U8*)"T_META_EOT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
     return;
    }break;

    case T_META_CUEPOINT:{
     amTrace((const U8*)"T_META_CUEPOINT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
     return;
    }break;

    case T_META_MARKER:{
     amTrace((const U8*)"T_META_MARKER: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
     return;
    }break;

    case T_META_SET_SIGNATURE:{
     amTrace((const U8*)"T_META_SET_SIGNATURE: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
     return;
    }break;

    case T_SYSEX:{
     amTrace((const U8*)"T_SYSEX: block pointer: %p, function pointer: %p\n",pPtr,pPtr->sendEventCb.func);
    return;

    }break;

    default:{
          amTrace((const U8*)"printEventBlock() error unknown/unsupported event type %d\n",(U16)(pPtr->type));
          return;
    }
    break;
 }
#endif


  /* decode stored event */
return;	
}


