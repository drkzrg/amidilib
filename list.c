
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
    
#include <assert.h>
#include <string.h>
#include "include/amidilib.h"
#include "include/amlog.h"
#include "include/list/list.h"
#include "include/mfp.h"

void initEventList(sEventList *listPtr){
	/*assert(listPtr!=NULL);*/
	listPtr=NULL;

}

/* adds event to linked list, list has to be inintialised with initEventList() function */
U32 addEvent(sEventList *listPtr, sEventBlock_t *eventBlockPtr ){
 
 sEventList *pTempPtr=NULL;
 sEventList *pNewItem=NULL;
 
 U32 uiDeltaNew=0;

	if((listPtr!=NULL)){
		/* list not empty, start at very first element */
		/* find the element with higher delta and insert item */
		
		pTempPtr=listPtr;
		uiDeltaNew=eventBlockPtr->uiDeltaTime;
			
			
		while((pTempPtr->pNext != NULL)){
		  pTempPtr=pTempPtr->pNext;
		}

		/* insert event */

			if(pTempPtr->pNext==NULL){
				/* insert at the end of list */
				pNewItem=(sEventList *)malloc(sizeof(sEventList));
				/*assert(pNewItem>0);*/					
				/* assert malloc ok, TODO: proper disaster handling */

				/* add data to new list node */
				(*pNewItem).eventBlock.uiDeltaTime = eventBlockPtr->uiDeltaTime;
				(*pNewItem).eventBlock.type = eventBlockPtr->type;
				(*pNewItem).eventBlock.infoBlock.size = eventBlockPtr->infoBlock.size;
				(*pNewItem).eventBlock.infoBlock.func=eventBlockPtr->infoBlock.func;
				(*pNewItem).eventBlock.dataPtr=NULL;
		
				/* allocate memory for event data and copy them to the new destination */
				(*pNewItem).eventBlock.dataPtr = malloc( ((*eventBlockPtr).infoBlock.size * sizeof(U8)));
				memcpy((void *)((*pNewItem).eventBlock.dataPtr),eventBlockPtr->dataPtr,((*eventBlockPtr).infoBlock.size * sizeof(U8)));

				pNewItem->pNext=NULL;					/* next node is NULL for new node */
				pNewItem->pPrev=pTempPtr;				/* prev node is current element node */
				
				/* add newly created list node to our list */
				pTempPtr->pNext=pNewItem;
				
				return 0;
			}
			else{
				/* insert between current and next one */
				pNewItem=(sEventList *)malloc(sizeof(sEventList));
				/*assert(pNewItem>0);*/						/* assert malloc ok, TODO: proper disaster handling */

				/* add data to new list node */
				pNewItem->eventBlock.uiDeltaTime = eventBlockPtr->uiDeltaTime;
				pNewItem->eventBlock.type = eventBlockPtr->type;
				pNewItem->eventBlock.infoBlock.size = eventBlockPtr->infoBlock.size;
				pNewItem->eventBlock.infoBlock.func=eventBlockPtr->infoBlock.func;
				pNewItem->eventBlock.dataPtr=NULL;
		
				/* allocate memory for event data and copy them to the new destination */
				pNewItem->eventBlock.dataPtr = malloc( eventBlockPtr->infoBlock.size * sizeof(U8));
				memcpy(pNewItem->eventBlock.dataPtr,eventBlockPtr->dataPtr,eventBlockPtr->infoBlock.size * sizeof(U8));

				/* set up pointers */
				pNewItem->pNext=pTempPtr->pNext;					/* new element points to next node */
				pNewItem->pPrev=pTempPtr;							/* prev node is current element node */

				/* insert between the two nodes */
				pTempPtr->pNext->pPrev=pNewItem;					/* next item points to new element */
				pTempPtr->pNext=pNewItem;							/* current node points to new item */

				return 0;
			}

		
	}
	else {
		/* add first element */
		listPtr=(sEventList *)malloc(sizeof(sEventList));
		listPtr->eventBlock.uiDeltaTime=eventBlockPtr->uiDeltaTime;
		listPtr->eventBlock.type = eventBlockPtr->type;
		listPtr->eventBlock.infoBlock.size = eventBlockPtr->infoBlock.size;
		listPtr->eventBlock.infoBlock.func=eventBlockPtr->infoBlock.func;
		listPtr->eventBlock.dataPtr=NULL;
		
		/* allocate memory for event data and copy them to the new destination */
		listPtr->eventBlock.dataPtr = malloc( eventBlockPtr->infoBlock.size * sizeof(U8));
		memcpy(listPtr->eventBlock.dataPtr,eventBlockPtr->dataPtr,eventBlockPtr->infoBlock.size * sizeof(U8));

		listPtr->pPrev=NULL;	/* first element in the list, no previous item */
		listPtr->pNext=NULL;
		
		return 0;
	}
}

U32 destroyList(sEventList *listPtr)
{
	sEventList *pTemp=NULL,*pCurrentPtr=NULL;
	
	if(listPtr!=NULL){
	
	  /*go to the end of the list */
	  pTemp=listPtr;
			
	  while(pTemp->pNext!=NULL){
	    pTemp=pTemp->pNext;
	  }
			
	  /* we are at the end of list, rewind to the previous element */
	  pCurrentPtr=pTemp->pPrev;
			
	  /* iterate to the begining */
			while(pCurrentPtr!=NULL){
				
			  if(((pCurrentPtr->eventBlock.dataPtr)>(void *)(0L))){
			    free(pCurrentPtr->eventBlock.dataPtr);
			    pCurrentPtr->eventBlock.dataPtr=NULL;
			  }

				free(pCurrentPtr->pNext);
				pCurrentPtr->pNext=NULL;
				pCurrentPtr=pCurrentPtr->pPrev;
			}
			
			/* we are at first element */
			/* remove it */
			free(listPtr);
			listPtr=NULL;
			
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
		pTemp=(*listPtr);

		
		while(pTemp!=NULL){
			/* print */
			const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
			printEventBlock(counter,pBlock);		
			counter++;
			pTemp=pTemp->pNext;
		}

		
	}
}

/* handles event (TODO: send all events with given delta, get next delta, set timer)  */

void printEventBlock(U32 counter,volatile sEventBlockPtr_t pPtr){
 
   evntFuncPtr myFunc=NULL; 

   amTrace((const U8*)"delta: %d\n",(unsigned int)pPtr->uiDeltaTime);
   amTrace((const U8*)"event type: %d\n",pPtr->type);
   amTrace((const U8*)"function pointer: %p\n",pPtr->infoBlock.func);
   amTrace((const U8*)"data size: %u\n",pPtr->infoBlock.size);
   amTrace((const U8*)"data pointer: %p\n",pPtr->dataPtr);
   
   amTrace((const U8*)"data: \t");
   U8 *pbuf=(U8 *)pPtr->dataPtr;
   for(int x=0;x<pPtr->infoBlock.size;x++){
    amTrace((const U8*)"0x%x \t",pbuf[x]);
   }
   amTrace((const U8*)"\n");
   
   
    switch((U16)(pPtr->type)){
    case T_NOTEON:{			  
      amTrace((const U8*)"T_NOTEON: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
      myFunc=pPtr->infoBlock.func;
      (*myFunc)(pPtr->dataPtr);}						  
     break;
			  
      case T_NOTEOFF:{
	amTrace((const U8*)"T_NOTEOFF: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	myFunc=pPtr->infoBlock.func;
	(*myFunc)((void *)pPtr->dataPtr);}
       break;
			  
      case T_NOTEAFT:{
	  amTrace((const U8*)"T_NOTEAFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);

	  myFunc=pPtr->infoBlock.func;
	  (*myFunc)((void *)pPtr->dataPtr);}
      break;
			  
      case T_CONTROL:{
	  amTrace((const U8*)"T_CONTROL: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);

	   myFunc=pPtr->infoBlock.func;
	    (*myFunc)((void *)pPtr->dataPtr);}
	break;
			  
	case T_PRG_CH:{
	    amTrace((const U8*)"T_PRG_CH: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	    myFunc=pPtr->infoBlock.func;
	    (*myFunc)((void *)pPtr->dataPtr);}
	break;
	case T_CHAN_AFT:{
	    amTrace((const U8*)"T_CHAN_AFT: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	    myFunc=pPtr->infoBlock.func;
	    (*myFunc)((void *)pPtr->dataPtr);}
	break;
			  
	case T_PITCH_BEND:{
	      amTrace((const U8*)"T_PITCH_BEND: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	      myFunc=pPtr->infoBlock.func;
	      (*myFunc)((void *)pPtr->dataPtr);}
	break;
	case T_META_SET_TEMPO:{
	      amTrace((const U8*)"T_META_SET_TEMPO: block pointer: %p, function pointer: %p\n",pPtr,pPtr->infoBlock.func);
	      myFunc=pPtr->infoBlock.func;
	      (*myFunc)((void *)pPtr->dataPtr);}
	break;	
	default:
	      amTrace((const U8*)"printEventBlock() error unknown event type %d\n",(U16)(pPtr->type));
	break;
 }  
		
		/* decode stored event */
	
}

U32 sendMidiEvents(U32 delta_start, const sEventList **listPtr)
{
	sEventList *pTemp=NULL;	
	U32 counter=0;
		
	/*assert(listPtr!=NULL);*/

	if((*listPtr)!=NULL){
		/* iterate through list */
		pTemp=(*listPtr);

		/* find first event with given delta */

		while(( (delta_start!=(*pTemp).eventBlock.uiDeltaTime)&&(pTemp!=NULL)) )
		{
				pTemp=pTemp->pNext;
		}
		
		while(( (pTemp!=NULL)&&(pTemp->eventBlock.uiDeltaTime==delta_start))){
			
		  /* send all events with given delta  */
			const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
			//printEventBlock(counter,&pBlock);
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

