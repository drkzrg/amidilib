
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/midiseq.h"
#include "include/amidiseq.h"
#include "include/midi_cmd.h"

extern volatile sSequence_t *pCurrentSequence;	//here is stored current sequence


void  fNoteOn(void *pEvent){
	sNoteOn_EventBlock_t *pPtr=(sNoteOn_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
	amTrace((const U8*)"Sending Note On data to sequencer ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
#endif	
	note_on(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);	
}

 void  fNoteOff(void *pEvent){
	sNoteOff_EventBlock_t *pPtr=(sNoteOff_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
 	amTrace((const U8*)"Sending Note Off data to sequencer ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
#endif	
	note_off(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

 void  fNoteAft(void *pEvent){
	sNoteAft_EventBlock_t *pPtr=(sNoteAft_EventBlock_t *)pEvent;	
#ifdef DEBUG_BUILD
	amTrace((const U8*)"Sending Note Aftertouch data to sequencer ch:%d note:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
#endif	
	polyphonic_key_press(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
}

 void  fProgramChange (void *pEvent){
	sPrgChng_EventBlock_t *pPtr=(sPrgChng_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
	amTrace((const U8*)"Sending Program change data to sequencer ch:%d pn:%d...\n",pPtr->ubChannelNb,pPtr->eventData.programNb);
#endif
	program_change(pPtr->ubChannelNb,pPtr->eventData.programNb);
}

 void  fController(void *pEvent){
	sController_EventBlock_t *pPtr=(sController_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
	amTrace((const U8*)"Sending Controller data to sequencer ch:%d controller:%d value:%d...\n",pPtr->ubChannelNb,pPtr->eventData.controllerNb,pPtr->eventData.value);
#endif
	control_change(pPtr->eventData.controllerNb, pPtr->ubChannelNb, pPtr->eventData.value,0x00);
}

 void  fChannelAft(void *pEvent){
	sChannelAft_EventBlock_t *pPtr=(sChannelAft_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
	amTrace((const U8*)"Sending Channel Aftertouch data to sequencer ch:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.pressure);
#endif
	channel_pressure (pPtr->ubChannelNb,pPtr->eventData.pressure);
}

 void  fPitchBend(void *pEvent){
	sPitchBend_EventBlock_t *pPtr=(sPitchBend_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
	amTrace((const U8*)"Sending Pitch bend data to sequencer ch:%d LSB:%d MSB:%d...\n",pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
#endif
	pitch_bend_2 (pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
}


 void fSetTempo(void *pEvent){
  sTempo_EventBlock_t *pPtr=(sTempo_EventBlock_t *)pEvent;	
 #ifdef DEBUG_BUILD
  amTrace((const U8*)"Setting new replay tempo...\n");
#endif
  
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    pCurrentSequence->arTracks[activeTrack]->currentState.newTempo=pPtr->eventData.tempoVal;
  }
}

 void fHandleEOT(void *pEvent){
  sEot_EventBlock_t *pPtr=(sEot_EventBlock_t *)pEvent;	
#ifdef DEBUG_BUILD
  amTrace((const U8*)"End of track...\n");
#endif
}
 
 void fHandleCuePoint(void *pEvent){
  sCuePoint_EventBlock_t *pPtr=(sCuePoint_EventBlock_t *)pEvent;	
#ifdef DEBUG_BUILD
  amTrace((const U8*)"Cuepoint %s.\n",pPtr->pCuePointName);
#endif
  
  
} 

 void fHandleMarker(void *pEvent){
  sMarker_EventBlock_t *pPtr=(sMarker_EventBlock_t *)pEvent;	
#ifdef DEBUG_BUILD
  amTrace((const U8*)"Marker %s.\n",pPtr->pMarkerName);
#endif
} 

 void fHandleSysEX(void *pEvent){
  sSysEX_EventBlock_t *pPtr=(sSysEX_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
  amTrace((const U8*)"SysEX Message.\n");
#endif
   
}


/* event id is mapped to the position in the array, functionPtr, parameters struct */

/** !ORDER IS IMPORTANT! and has to be the same as in enums with T_EVT_COUNT. Additionally
the ordering of members should be the same as described in type sEventList. */

static const sEventInfoBlock_t g_arSeqCmdTable[T_EVT_COUNT] = {
  {sizeof(sNoteOn_EventBlock_t),fNoteOn},
   {sizeof(sNoteOff_EventBlock_t),fNoteOff},
   {sizeof(sNoteAft_EventBlock_t), fNoteAft},
   {sizeof(sController_EventBlock_t),fController},
   {sizeof(sPrgChng_EventBlock_t),fProgramChange},
   {sizeof(sChannelAft_EventBlock_t),fChannelAft},
   {sizeof(sPitchBend_EventBlock_t),fPitchBend},
   {sizeof(sTempo_EventBlock_t),fSetTempo},
   {sizeof(sEot_EventBlock_t),fHandleEOT},
   {sizeof(sCuePoint_EventBlock_t),fHandleCuePoint},
   {sizeof(sMarker_EventBlock_t),fHandleMarker},
   {sizeof(sSysEX_EventBlock_t),fHandleSysEX}
};

/*returns pointer to NULL terminated string with event name */
/* id is enumerated value from eEventType */
const U8 *getEventName(U32 id){
  if(id<T_EVT_COUNT)
	return ((const U8 *)g_arEventNames[id]);
  else return 0;
}

void getEventFuncInfo(U8 eventType, sEventInfoBlock_t *infoBlk){
	infoBlk->size=g_arSeqCmdTable[eventType].size;
	infoBlk->func=g_arSeqCmdTable[eventType].func;
  
}
