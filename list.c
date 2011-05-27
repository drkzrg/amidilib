
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

#include "include/amidilib.h"
#include "include/amlog.h"
#include "include/list/list.h"
#include "include/mfp.h"

void initEventList(sEventList **listPtr){
  #ifdef DEBUG_MEM
    amTrace((const U8 *)"initEventList()\n");
  #endif
	/*assert(listPtr!=NULL);*/
	(*listPtr)=NULL;
}

/* adds event to linked list, list has to be inintialised with initEventList() function */
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

/* insert event */
  #ifdef DEBUG_MEM
    amTrace((const U8 *)"insert event\n");
  #endif
    
  /* insert at the end of list */
  copyEvent(eventBlockPtr, &pNewItem);
	
  pNewItem->pNext=NULL;		/* next node is NULL for new node */
  pNewItem->pPrev=pTempPtr;	/* prev node is current element node */
		  
  /* add newly created list node to our list */
  pTempPtr->pNext=pNewItem;
  
}else{
  #ifdef DEBUG_MEM
    amTrace((const U8 *)"insert first event\n");
  #endif
  
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
    (*dest)->eventBlock.infoBlock.size = src->infoBlock.size;
    (*dest)->eventBlock.infoBlock.func=src->infoBlock.func;
    (*dest)->eventBlock.dataPtr=NULL;
		
    /* allocate memory for event data and copy them to the new destination */
    (*dest)->eventBlock.dataPtr = amMallocEx( src->infoBlock.size * sizeof(U8),PREFER_TT);
    amMemCpy((*dest)->eventBlock.dataPtr,src->dataPtr,src->infoBlock.size * sizeof(U8));
    
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
	    amFree(&(pCurrentPtr->eventBlock.dataPtr));
	  }

	  amFree((void **)&(pCurrentPtr->pNext));
	  pCurrentPtr=pCurrentPtr->pPrev;
	}
	/* we are at first element */
	/* remove it */
	amFree((void **)listPtr);
	return 0;
  }
return 0;
}

void printEventList(const sEventList **listPtr){
	sEventList *pTemp=NULL;	

	U32 counter=0;
		
	/*assert(listPtr!=NULL);*/

	if((*listPtr)!=NULL){
		/* iterate through list */
		pTemp=(sEventList *)(*listPtr);

		
		while(pTemp!=NULL){
			/* print */
			const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
			printEventBlock(pBlock);		
			counter++;
			pTemp=pTemp->pNext;
		}

		
	}
}

/* prints event data and sends it to device (no timing)  */
void printEventBlock(sEventBlockPtr_t pPtr){
 
   evntFuncPtr myFunc=NULL; 
   U8 *pbuf=NULL;
   int x=0;

   amTrace((const U8*)"+++++++++++++ event info: \n");
   amTrace((const U8*)"delta: %d\t",(unsigned int)pPtr->uiDeltaTime);
   amTrace((const U8*)"event type: %d\t",pPtr->type);
   amTrace((const U8*)"function pointer: %p\t",pPtr->infoBlock.func);
   amTrace((const U8*)"data size: %u\t",pPtr->infoBlock.size);
   amTrace((const U8*)"data pointer: %p\n",pPtr->dataPtr);
   
   amTrace((const U8*)"data: \t");
   
   pbuf=(U8 *)pPtr->dataPtr;
   
   for(x=0;x<pPtr->infoBlock.size;x++){
    amTrace((const U8*)"0x%x ",pbuf[x]);
   }

   amTrace((const U8*)"\n");
    switch((U16)(pPtr->type)){
    case T_NOTEON:{			  
      amTrace((const U8*)"T_NOTEON: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
     }						  
     break;
			  
      case T_NOTEOFF:{
	  amTrace((const U8*)"T_NOTEOFF: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	}
       break;
			  
      case T_NOTEAFT:{
	  amTrace((const U8*)"T_NOTEAFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
      }
      break;
			  
      case T_CONTROL:{
	  amTrace((const U8*)"T_CONTROL: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
      }
      break;
			  
	case T_PRG_CH:{
	    amTrace((const U8*)"T_PRG_CH: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	}
	  break;
	case T_CHAN_AFT:{
	    amTrace((const U8*)"T_CHAN_AFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	}
	  	break;
			  
	case T_PITCH_BEND:{
	      amTrace((const U8*)"T_PITCH_BEND: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	}
	  break;
	case T_META_SET_TEMPO:{
	      amTrace((const U8*)"T_META_SET_TEMPO: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	}
	  break;	
	default:
	      amTrace((const U8*)"printEventBlock() error unknown event type %d\n",(U16)(pPtr->type));
	break;
 }  
  /* decode stored event */
	
}


// sends midi events
U32 sendMidiEvents(U32 delta_start, const sEventList **listPtr){
	sEventList *pTemp=NULL;	
	U32 counter=0;
		
	/*assert(listPtr!=NULL);*/

	if((*listPtr)!=NULL){
		/* iterate through list */
		pTemp=(sEventList *)(*listPtr);

		/* find first event with given delta */

		while(( (delta_start!=(*pTemp).eventBlock.uiDeltaTime)&&(pTemp!=NULL)) )
		{
				pTemp=pTemp->pNext;
		}
		
		while(( (pTemp!=NULL)&&(pTemp->eventBlock.uiDeltaTime==delta_start))){
			
		  /* send all events with given delta  */
			const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
			//printEventBlock(&pBlock);
			counter++;
			pTemp=pTemp->pNext;
		}
		
		if(pTemp==NULL) {
		 /* end of track next delta will be 0*/
		 return 0;
		}
		else{	
		 /* return next delta */
		 return ((pTemp->eventBlock.uiDeltaTime));
		}
		
	}
	return 0;
}

