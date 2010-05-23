/**  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/
    
#include <assert.h>
#include <string.h>
#include "INCLUDE/AMIDILIB.H"
#include "INCLUDE/LIST/LIST.H"
#include "INCLUDE/MFP.H"

void initEventList(sEventList *listPtr)
{
	/*assert(listPtr!=NULL);*/
	listPtr->pEventList=NULL;
	listPtr->uiNbOfItems=0;
}

/* adds event to linked list, list has to be inintialised with initEventList() function */
/* adds event to linked list, list has to be inintialised with initEventList() function */
U32 addEvent(sEventList *listPtr, sEventBlock_t *eventBlockPtr ){
 sEventItem *pTempPtr=NULL;
 sEventItem *pNewItem=NULL;
 U32 uiDeltaNew=0;

	if((listPtr->pEventList!=NULL)){
			/* list not empty, start at very first element */
			/* find the element with higher delta and insert item */
		
			pTempPtr=listPtr->pEventList;
			
			uiDeltaNew=eventBlockPtr->uiDeltaTime;
			
			
			while(((pTempPtr->pNext != NULL) && (pTempPtr->pNext->eventBlock.uiDeltaTime<=uiDeltaNew)) ){
				pTempPtr=pTempPtr->pNext;
			}

			/* insert event */

			if(pTempPtr->pNext==NULL){
				/* insert at the end of list */
				pNewItem=(sEventItem *)malloc(sizeof(sEventItem));
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
				listPtr->uiNbOfItems++;				/* increase event counter */
				return 0;
			}
			else
			{
				/* insert between current and next one */
				pNewItem=(sEventItem *)malloc(sizeof(sEventItem));
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

				/* increase list event counter */
				listPtr->uiNbOfItems++; /* increase event counter */
				return 0;
			}

		
	}
	else {
		/* add first element */
		listPtr->pEventList=(sEventItem *)malloc(sizeof(sEventItem));
		listPtr->pEventList->eventBlock.uiDeltaTime=eventBlockPtr->uiDeltaTime;
		listPtr->pEventList->eventBlock.type = eventBlockPtr->type;
		listPtr->pEventList->eventBlock.infoBlock.size = eventBlockPtr->infoBlock.size;
		listPtr->pEventList->eventBlock.infoBlock.func=eventBlockPtr->infoBlock.func;
		listPtr->pEventList->eventBlock.dataPtr=NULL;
		
		/* allocate memory for event data and copy them to the new destination */
		listPtr->pEventList->eventBlock.dataPtr = malloc( eventBlockPtr->infoBlock.size * sizeof(U8));
		memcpy(listPtr->pEventList->eventBlock.dataPtr,eventBlockPtr->dataPtr,eventBlockPtr->infoBlock.size * sizeof(U8));

		listPtr->pEventList->pPrev=NULL;	/* first element in the list, no previous item */
		listPtr->pEventList->pNext=NULL;
		
		/* increase event counter */
		listPtr->uiNbOfItems++;
		return 0;
	}
		

}

U32 destroyList(sEventList *listPtr)
{
	sEventItem *pTemp=NULL,*pCurrentPtr=NULL;
	/*assert(listPtr!=NULL);*/
	

		if(listPtr->pEventList!=NULL){
			
			/*go to the end of the list */
			pTemp=listPtr->pEventList;
			
			while(pTemp->pNext!=NULL){
				pTemp=pTemp->pNext;
			}
			
			/* we are at the end of list, rewind to the previous element */
			pCurrentPtr=pTemp->pPrev;
			
			/* iterate to the begining */
			while(pCurrentPtr!=NULL){
				
				if(pCurrentPtr->eventBlock.dataPtr>0)
				{
					free(pCurrentPtr->eventBlock.dataPtr);
					pCurrentPtr->eventBlock.dataPtr=NULL;
				}

				free(pCurrentPtr->pNext);
				pCurrentPtr->pNext=NULL;
				pCurrentPtr=pCurrentPtr->pPrev;
			}
			
			/* we are at first element */
			/* remove it */
			free(listPtr->pEventList);
			listPtr->pEventList=NULL;
			
			/* reset number of items */
			listPtr->uiNbOfItems=0;
			return 0;
		}
	
	
return 0;
}

void printEventList(const sEventList **listPtr){
	sEventItem *pTemp=NULL;	

	U32 counter=0;
		
	/*assert(listPtr!=NULL);*/

	if((*listPtr)->pEventList!=NULL){
		/* iterate through list */
		pTemp=(*listPtr)->pEventList;

		
		while(pTemp!=NULL)
		{
			/* print */
			const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
			printEventBlock(counter,&pBlock);		
			counter++;
			pTemp=pTemp->pNext;
		}

		
	}
}

/* handles event (TODO: send all events with given delta, get next delta, set timer)  */

void printEventBlock(U32 counter,const sEventBlockPtr_t *pPtr){
 
 			evntFuncPtr myFunc; 

  			 /* print delta */
			amTrace((const U8*)"event nb: %u event delta: %u\n",(unsigned int)counter,(unsigned int)(*pPtr)->uiDeltaTime);

			switch((U16)((*pPtr)->type)){
			  case T_NOTEON:			  
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((*pPtr)->dataPtr);						  
			  break;
			  
			  case T_NOTEOFF:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;
			  
			  case T_NOTEAFT:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;
			  
			  case T_CONTROL:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;
			  
			  case T_PRG_CH:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;
			  
			  case T_CHAN_AFT:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;
			  
			  case T_PITCH_BEND:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;
			  
			  case T_META_SET_TEMPO:
			  myFunc=(*pPtr)->infoBlock.func;
			  (*myFunc)((void *)(*pPtr)->dataPtr);
			  break;	
			
			}  
		
		/* decode stored event */
	
}


/* sends all midi events with given delta */
/* returns next delta time or 0, if end of track */
/* waits for space keypress after sending all of the data */

/* MIDI replay draft */
/*

int iCurrentDelta=0;
sEventItem *pTemp


*/
volatile extern sEventItem *pEvent;
volatile extern U32 iCurrentDelta;

void playSequence(const sEventList **listPtr){
  
  if((*listPtr)->pEventList!=NULL){
    pEvent=(*listPtr)->pEventList;
    iCurrentDelta=0;
    //getTempo 
    installMIDIreplay(MFP_DIV10,59);
  }
}


U32 sendMidiEvents(U32 delta_start, const sEventList **listPtr)
{
	sEventItem *pTemp=NULL;	
	U32 counter=0;
		
	/*assert(listPtr!=NULL);*/

	if((*listPtr)->pEventList!=NULL){
		/* iterate through list */
		pTemp=(*listPtr)->pEventList;

		/* find first event with given delta */

		while(( (delta_start!=(*pTemp).eventBlock.uiDeltaTime)&&(pTemp!=NULL)) )
		{
				pTemp=pTemp->pNext;
		}
		
		while(( (pTemp!=NULL)&&(pTemp->eventBlock.uiDeltaTime==delta_start))){
			
		  /* send all events with given delta  */
			const sEventBlockPtr_t pBlock=(const sEventBlockPtr_t)&pTemp->eventBlock;
			printEventBlock(counter,&pBlock);
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

