
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
    
#include <assert.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "amidilib.h"
#include "amlog.h"
#include "list/list.h"

#ifndef PORTABLE
#include "timing/mfp.h"
#endif

#include "memory/linalloc.h" /* custom memory allocator */

/* adds event to linked list, list has to be inintialised with null */
//event list, temp event

void addEvent(sEventList **listPtr, sEventBlock_t *eventBlockPtr ){
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
  copyEvent(eventBlockPtr, &pNewItem);
	
  pNewItem->pNext=NULL;		/* next node is NULL for new node */
  pNewItem->pPrev=pTempPtr;	/* prev node is current element node */
		  
  /* add newly created list node to our list */
  pTempPtr->pNext=pNewItem;
  
}else{
  
  copyEvent(eventBlockPtr, listPtr);
  
  (*listPtr)->pPrev=NULL;		/* first element in the list, no previous item */
  (*listPtr)->pNext=NULL;
 }
}

void copyEvent(const sEventBlock_t *src, sEventList **dest){
  #ifdef DEBUG_MEM
    amTrace((const U8 *)"copyEvent() src: %p dst: %p\n",src,dest);
  #endif
    
    (*dest)=(sEventList *)amMallocEx(sizeof(sEventList),PREFER_TT);
    
    (*dest)->eventBlock.uiDeltaTime=src->uiDeltaTime;
    (*dest)->eventBlock.type = src->type;
    (*dest)->eventBlock.sendEventCb.size = src->sendEventCb.size;
    (*dest)->eventBlock.sendEventCb.func=src->sendEventCb.func;
    (*dest)->eventBlock.copyEventCb.size = src->copyEventCb.size;
    (*dest)->eventBlock.copyEventCb.func=src->copyEventCb.func;
    
    (*dest)->eventBlock.dataPtr=NULL;
		
    /* allocate memory for event data and copy them to the new destination */
    (*dest)->eventBlock.dataPtr = amMallocEx((src->sendEventCb.size * sizeof(U8)),PREFER_TT);
    amMemCpy((*dest)->eventBlock.dataPtr,src->dataPtr,(src->sendEventCb.size * sizeof(U8)));
    
}

U32 destroyList(sEventList **listPtr){
sEventList *pTemp=NULL,*pCurrentPtr=NULL;

#ifdef DEBUG_MEM
amTrace((const U8 *)"destroyList()\n");
#endif
	
	if(*listPtr!=NULL){
	
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
		amFree((void **)&(pTemp->pMarkerName));
	      }break;
	      
	      case T_META_CUEPOINT:{
		sCuePoint_EventBlock_t *pTemp=(sCuePoint_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
		amFree((void **)&(pTemp->pCuePointName));
	      }break;

	      case T_SYSEX:{
		sSysEX_EventBlock_t *pTemp=(sSysEX_EventBlock_t *)pCurrentPtr->eventBlock.dataPtr;
		amFree((void **)&(pTemp->pBuffer));
	      }break;
	    };
	    //release event block itself
	    amFree(&(pCurrentPtr->eventBlock.dataPtr));
	  }

	  amFree((void **)&(pCurrentPtr->pNext));
	  pCurrentPtr=pCurrentPtr->pPrev;
	}
	/* we are at first element */
	/* remove it */
	amFree((void **)listPtr);
  }
return 0;
}

void printEventList(const sEventList *listPtr){
	sEventList *pTemp=NULL;	
	U32 counter=0;
		
	if(listPtr!=NULL){
		/* iterate through list */
		pTemp=(sEventList *)listPtr;
		
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
   amTrace((const U8*)"delta: %d\t",(unsigned int)pPtr->uiDeltaTime);
   amTrace((const U8*)"event type: %d\t",pPtr->type);
   amTrace((const U8*)"send event callback: %p\t",pPtr->sendEventCb.func);
   amTrace((const U8*)"send data size: %u\t",pPtr->sendEventCb.size);
   amTrace((const U8*)"copy event callback: %p\t",pPtr->copyEventCb.func);
   amTrace((const U8*)"copy data size: %u\t",pPtr->copyEventCb.size);
   amTrace((const U8*)"data pointer: %p\n",pPtr->dataPtr);
   amTrace((const U8*)"data: \t");
   
   pbuf=(U8 *)pPtr->dataPtr;
   
   for(x=0;x<pPtr->sendEventCb.size;x++){
    amTrace((const U8*)"0x%x ",pbuf[x]);
   }

   amTrace((const U8*)"\n");
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
  /* decode stored event */
return;	
}


