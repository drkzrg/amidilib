
#include "timing/miditim.h"
#include "mformats.h"
#include "config.h"
#include "containers/list.h"
#include "midi.h"
#include "mparser.h"

#include "string.h"

#ifdef MIDI_PARSER_DEBUG
#include "core/logger.h"
#include "core/amprintf.h"
#endif

/* for saving last running status */
static uint8 g_runningStatus=0;

//////////////////////////////////////////////////////
uint16 amGetTimeDivision (const void *pMidiPtr)
{
sMThd *pMidiInfo=(sMThd *)pMidiPtr;

/* check midi header */
if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L))){
  return (pMidiInfo->division);
}
 return (0);
}

/** checks if byte is midi channel
* @param byteEvent command byte
* @return 1 if true, 0 otherwise
*/

static INLINE Bool amIsMidiChannelEvent(const uint8 byteEvent)
{
  if(( ((byteEvent&0xF0)>=0x80) && ((byteEvent&0xF0)<=0xE0)))
  {
    return TRUE;
  }
    
  return FALSE;
}


/** checks if byte is Sysex or System realtime command byte
* @param byteEvent pointer to VLQ data
* @return 1 if true, 0 otherwise
*/

static INLINE Bool amIsMidiRtCmdOrSysex(const uint8 byteEvent)
{
  if( ((byteEvent>=(uint8)0xF0)&&(byteEvent<=(uint8)0xFF)) )
  {
      /* it is! */
      return TRUE;
   }
  
  /*no, it's not! */
  return FALSE;
}

/************************************************ midi event processing functions */

/** read/decode note off message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amNoteOff(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb)
{
sEventBlock_t tempEvent;
sNoteOff_EventBlock_t *pEvntBlock=NULL;
sNoteOff_t *pNoteOff=0;
tempEvent.dataPtr=0;
int16 retCode = AM_OK;

if((*recallRS)==0){
  /* save last running status */
  g_runningStatus=*(*pPtr);

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;

  getEventCallbackHandler(T_NOTEOFF,&tempEvent.eventCb);
  uint8 tempBuf[tempEvent.eventCb.size];

  tempEvent.dataPtr=(void *)tempBuf;

  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  /* now we can recall former running status next time */
  (*recallRS)=1;

  ++(*pPtr);
  pNoteOff=(sNoteOff_t *)(*pPtr);

  /* get parameters */
  pEvntBlock->eventData.noteNb=pNoteOff->noteNb;
  pEvntBlock->eventData.velocity=pNoteOff->velocity;

  (*pPtr)=(*pPtr)+sizeof(sNoteOff_t);

  /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
   retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
}else {
  /* recall last cmd status */
  /* and get parameters as usual */

  /* get last note info */
  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;

  getEventCallbackHandler(T_NOTEOFF,&tempEvent.eventCb);
  uint8 tempBuf[tempEvent.eventCb.size];

  tempEvent.dataPtr=(void *)tempBuf;

  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;

  /* save channel */
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  pNoteOff=(sNoteOff_t *)(*pPtr);

  /* get parameters */
  pEvntBlock->eventData.noteNb=pNoteOff->noteNb;
  pEvntBlock->eventData.velocity=pNoteOff->velocity;

  (*pPtr)=(*pPtr)+sizeof(sNoteOff_t);

  /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
   retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

}

#ifdef MIDI_PARSER_DEBUG
  amTrace("delta: %u\t",delta);
  amTrace("event: Note off ",0);
  amTrace("ch: %d\t",(pEvntBlock->ubChannelNb)+1);
  amTrace("note: %d(%s)\t",pNoteOff->noteNb,getMidiNoteName(pNoteOff->noteNb));
  amTrace("vel: %d"NL,pNoteOff->velocity);
#endif

   return retCode;
}

/** read/decode note on message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amNoteOn(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb)
{
 sEventBlock_t tempEvent;

 uint8 channel=0;
 uint8 note=0;
 uint8 velocity=0;
 int16 retCode = AM_OK;

 sNoteOn_EventBlock_t *pEvntBlock=NULL;
 tempEvent.dataPtr=0;

 if((*recallRS)==0){
    /* save last running status */
  g_runningStatus=*(*pPtr);

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEON;

  getEventCallbackHandler(T_NOTEON,&tempEvent.eventCb);
  uint8 tempBuf[tempEvent.eventCb.size];

  tempEvent.dataPtr=(void *) tempBuf;

  /*assert(tempEvent.dataPtr>0);*/
  pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;

  /* now we can recall former running status next time */
  (*recallRS)=1;

  ++(*pPtr);
  channel=(g_runningStatus&0x0F)+1;
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
  note=*(*pPtr);
  pEvntBlock->eventData.noteNb=*(*pPtr);

  /* get parameters */
  ++(*pPtr);
  velocity=*(*pPtr);
  pEvntBlock->eventData.velocity=*(*pPtr);

  ++(*pPtr);

  /* add event to list */
  #ifdef EVENT_LINEAR_BUFFER
    retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
  #else
     retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
  #endif

 }else{
    /* get last note info */
    channel=(g_runningStatus&0x0F)+1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEON;
    
    getEventCallbackHandler(T_NOTEON,&tempEvent.eventCb);
    uint8 tempBuf[tempEvent.eventCb.size];

    tempEvent.dataPtr=(void *) tempBuf;

    pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    note=*(*pPtr);
    pEvntBlock->eventData.noteNb=*(*pPtr);
    /* get parameters */
    ++(*pPtr);
    velocity=*(*pPtr);
    pEvntBlock->eventData.velocity=*(*pPtr);

    ++(*pPtr);

    /* add event to list */
    #ifdef EVENT_LINEAR_BUFFER
      retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
    #else
       retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
    #endif

 }

 #ifdef MIDI_PARSER_DEBUG
    amTrace("delta: %lu\t",delta);
    amTrace("event: Note on ",0);
    amTrace("ch: %d\t",channel);
    amTrace("note: %d(%s)\t",note,getMidiNoteName(note));
    amTrace("vel: %d "NL,velocity);
 #endif

 return retCode;
}

/** read/decode note aftertouch/pressure message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amNoteAft(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb)
{
sEventBlock_t tempEvent;
uint8 noteNb=0;
uint8 pressure=0;
int16 retCode = AM_OK;

sNoteAft_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0)
 {
    /* save last running status */
    g_runningStatus=*(*pPtr);

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEAFT;

    getEventCallbackHandler(T_NOTEAFT,&tempEvent.eventCb);
    uint8 tempBuf[tempEvent.eventCb.size];
    tempEvent.dataPtr=(void *) tempBuf;

    pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    /* now we can recall former running status next time */
    (*recallRS)=1;

    ++(*pPtr);
    /* get parameters */
    noteNb=*(*pPtr);
    pEvntBlock->eventData.noteNb=*(*pPtr);
    ++(*pPtr);
    pressure=*(*pPtr);
    pEvntBlock->eventData.pressure=*(*pPtr);
    ++(*pPtr);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
   retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

 }else{
        /* get parameters */
    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEAFT;

    getEventCallbackHandler(T_NOTEAFT,&tempEvent.eventCb);
    uint8 tempBuf[tempEvent.eventCb.size];
    tempEvent.dataPtr=(void *) tempBuf;

    pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    noteNb=*(*pPtr);
    pEvntBlock->eventData.noteNb=*(*pPtr);
    ++(*pPtr);
    pressure=*(*pPtr);
    pEvntBlock->eventData.pressure=*(*pPtr);
    ++(*pPtr);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
   retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
    }

#ifdef MIDI_PARSER_DEBUG
     amTrace("delta: %u\tevent: Note Aftertouch note: %d, pressure: %d"NL,(unsigned long)delta, noteNb,pressure);
#endif

   return retCode;
}

/** read/decode control change message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amController(sSequence_t *pSeq,uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb)
{
    sEventBlock_t tempEvent;
    int16 retCode = AM_OK;
    uint8 channelNb=0;
    uint8 controllerNb=0;
    uint8 value=0;
    sController_EventBlock_t *pEvntBlock=NULL;
    tempEvent.dataPtr=0;

    if((*recallRS)==0)
    {
      /* save last running status */
      g_runningStatus=*(*pPtr);
      /* now we can recall former running status next time */
      (*recallRS)=1;

      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_CONTROL;

      getEventCallbackHandler(T_CONTROL,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];
      tempEvent.dataPtr=(void *) tempBuf;

      pEvntBlock=(sController_EventBlock_t *)tempEvent.dataPtr;

      channelNb=g_runningStatus&0x0F;
      pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
      ++(*pPtr);
      /* get controller nb */
      controllerNb=(*(*pPtr));
      pEvntBlock->eventData.controllerNb=(*(*pPtr));
      ++(*pPtr);
      value=*((*pPtr));
      pEvntBlock->eventData.value=*((*pPtr));
      ++(*pPtr);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

    } 
    else 
    {
      channelNb=g_runningStatus&0x0F;
      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_CONTROL;

      getEventCallbackHandler(T_CONTROL,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr=(void *) tempBuf;

      /*assert(tempEvent.dataPtr>0);*/
      pEvntBlock=(sController_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

      /* get program controller nb */
      controllerNb=(*(*pPtr));
      pEvntBlock->eventData.controllerNb=(*(*pPtr));
      ++(*pPtr);
      value=*((*pPtr));
      pEvntBlock->eventData.value=*((*pPtr));
      ++(*pPtr);
      /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
  }
#ifdef MIDI_PARSER_DEBUG
    amTrace("delta: %lu\tevent: Controller ch: %d, nb:%d name: %s\tvalue: %d"NL,delta, channelNb+1, controllerNb,getMidiControllerName(controllerNb), value);
#endif

   return retCode;
}

/** read/decode program change message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amProgramChange(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta,  const uint8 trackNb)
{
sEventBlock_t tempEvent;

  uint8 channel=0;
  uint8 PN=0;
  int16 retCode = AM_OK;

  sPrgChng_EventBlock_t *pEvntBlock=NULL;
  tempEvent.dataPtr=0;

  if((*recallRS)==0)
  {
    /* save last running status */
    g_runningStatus=*(*pPtr);

    /* now we can recall former running status next time */
    (*recallRS)=1;

    channel=(g_runningStatus&0x0F)+1;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_PRG_CH;

    getEventCallbackHandler(T_PRG_CH,&tempEvent.eventCb);
    uint8 tempBuf[tempEvent.eventCb.size];

    tempEvent.dataPtr=(void *) tempBuf;

    pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
    ++(*pPtr);
    /* get parameters */
    PN=*(*pPtr);
    pEvntBlock->eventData.programNb=*(*pPtr);
    ++(*pPtr);

    /* add event to list */
    #ifdef EVENT_LINEAR_BUFFER
      retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
    #else
      retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
    #endif

    } else {
      /* get last PC status */
      channel=(g_runningStatus&0x0F)+1;
      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_PRG_CH;

      getEventCallbackHandler(T_PRG_CH,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr=(void *) tempBuf;

      pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

      PN=*(*pPtr);
      pEvntBlock->eventData.programNb=*(*pPtr);

     /* get parameters */
      ++(*pPtr);

      /* add event to list */
      #ifdef EVENT_LINEAR_BUFFER
        retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
      #else
        retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
      #endif

     }

#ifdef MIDI_PARSER_DEBUG
    amTrace("delta: %lu\t",delta);
    amTrace("event: Program change ",0);
    amTrace("ch: %d\t",channel);
    amTrace("program nb: %d"NL,PN);
#endif

    return retCode;
 }


/** read/decode channel aftertouch/pressure message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amChannelAft(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb)
{
sEventBlock_t tempEvent;
int16 retCode = AM_OK;

    uint8 param=0;
    sChannelAft_EventBlock_t *pEvntBlock=NULL;
    tempEvent.dataPtr=0;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

        tempEvent.uiDeltaTime=delta;
        tempEvent.type=T_CHAN_AFT;

        getEventCallbackHandler(T_CHAN_AFT,&tempEvent.eventCb);
        uint8 tempBuf[tempEvent.eventCb.size];

        tempEvent.dataPtr=(void *)tempBuf;

        pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
        pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        ++(*pPtr);
        param=*(*pPtr);
        pEvntBlock->eventData.pressure=*(*pPtr);
        ++(*pPtr);

        /* add event to list */
        #ifdef EVENT_LINEAR_BUFFER
            retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
        #else
            retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
        #endif

    } else {
        tempEvent.uiDeltaTime=delta;
        tempEvent.type=T_CHAN_AFT;

        getEventCallbackHandler(T_CHAN_AFT,&tempEvent.eventCb);
        uint8 tempBuf[tempEvent.eventCb.size];

        tempEvent.dataPtr=(void *)tempBuf;

        pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
        pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        param=*(*pPtr);
        pEvntBlock->eventData.pressure=*(*pPtr);
        ++(*pPtr);

        /* add event to list */
        #ifdef EVENT_LINEAR_BUFFER
            retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
        #else
            retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
        #endif
    }

#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %lu\tevent: Channel aftertouch pressure: %d"NL,delta, param);
#endif

 return retCode;
}

/** read/decode pitch bend message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amPitchBend(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb)
{
sEventBlock_t tempEvent;

sPitchBend_EventBlock_t *pEvntBlock=NULL;
sPitchBend_t *pPitchBend=0;
int16 retCode = AM_OK;

tempEvent.dataPtr=0;

    if((*recallRS)==0)
    {
      /* save last running status */
      g_runningStatus=*(*pPtr);

      /* now we can recall former running status next time */
      (*recallRS)=1;

      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_PITCH_BEND;

      getEventCallbackHandler(T_PITCH_BEND,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr=(void *) tempBuf;

      pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

      ++(*pPtr);
      pPitchBend=(sPitchBend_t *)(*pPtr);

      /* get parameters */
      pEvntBlock->eventData.LSB=pPitchBend->LSB;
      pEvntBlock->eventData.MSB=pPitchBend->MSB;
      (*pPtr)=(*pPtr)+sizeof(sPitchBend_t);

      /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

    } 
    else
    {
      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_PITCH_BEND;

      getEventCallbackHandler(T_PITCH_BEND,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr = (void *)tempBuf;

      pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

      /* get parameters */
      pPitchBend=(sPitchBend_t *)(*pPtr);

      /* get parameters */
      pEvntBlock->eventData.LSB=pPitchBend->LSB;
      pEvntBlock->eventData.MSB=pPitchBend->MSB;
      (*pPtr)=(*pPtr)+sizeof(sPitchBend_t);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      retCode = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      retCode = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
  }

 #ifdef MIDI_PARSER_DEBUG
 amTrace("delta: %u\tevent: Pitch bend LSB: %d, MSB:%d"NL,(unsigned long)delta,pPitchBend->LSB,pPitchBend->MSB);
 #endif

  return retCode;
}

/** read/decode sysex message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
* @return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amSysexMsg(sSequence_t *pSeq, uint8 **pPtr, const uint32 delta, const uint8 trackNb)
{
  sEventBlock_t tempEvent;
  sSysEX_EventBlock_t *pEvntBlock=0;
  uint8 *pTmpPtr=0;

  uint32 ulCount=0L;
  tempEvent.dataPtr=0;

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_SYSEX;

  getEventCallbackHandler(T_SYSEX,&tempEvent.eventCb);
  uint8 tempBuf[tempEvent.eventCb.size];

  tempEvent.dataPtr=(void *) tempBuf;

  pEvntBlock=(sSysEX_EventBlock_t *)tempEvent.dataPtr;

#ifdef MIDI_PARSER_DEBUG
  amTrace("SOX: ",0);
#endif
   pTmpPtr=(*pPtr); //save start

   while( (*(*pPtr))!=EV_EOX){
#ifdef MIDI_PARSER_DEBUG
    amTrace("0x%x, ",*(*pPtr));
#endif
     (*pPtr)++;
      /*count Sysex msg data bytes */
      ulCount++;
    }
    pEvntBlock->bufferSize=ulCount; //size of data
    pEvntBlock->pBuffer = (uint8 *)gUserMemAlloc(ulCount*sizeof(uint8),PREFER_TT,0);

    //copy ulCount of data
    amMemCpy(pEvntBlock->pBuffer,pTmpPtr,ulCount*sizeof(uint8));

#ifdef MIDI_PARSER_DEBUG
    amTrace(" EOX, size: %lu"NL,ulCount);
#endif

 /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  return addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
   return addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
}

/** read/decode meta event message
* @param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*   @return Bool flag TRUE if end of track (EOT) occured, FALSE otherwise. iRetVal pointer to an integer, 
*           which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/

static INLINE int16 amMetaEvent(sSequence_t *pSeq, uint8 **pPtr, const uint32 delta, const uint8 trackNb, Bool *bEOT)
{
sEventBlock_t tempEvent;

 uint32 addr;
 uint8 ubLenght,ubVal,ubSize=0;
 uint8 textBuffer[64]={0};
 *bEOT=FALSE;
 tempEvent.dataPtr = 0;
 int16 ret = AM_OK;

 /*get meta event type */
 (*pPtr)++;
 ubVal=*(*pPtr);

 switch(ubVal)
 {
    case MT_SEQ_NB:
    {
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Sequence nb: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
#ifdef MIDI_PARSER_DEBUG
        amTrace("%d"NL, ubLenght);
#endif
        (*pPtr)++;
        addr=((uint32)(*pPtr))+ubLenght*sizeof(uint8);
        *pPtr=(uint8*)addr;
      ret = AM_OK;
    } break;
    case MT_TEXT:
    {
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Text:",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace("meta size: %d ",ubLenght);
#endif
    /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(uint8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;
    case MT_COPYRIGHT:
    {
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Copyright: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace("meta size: %d ",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(uint8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;
    case MT_SEQNAME:
    {
#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %u\tMeta event: Sequence name: ",(unsigned long)delta);
#endif
      ++(*pPtr);
      ubLenght=readVLQ((*pPtr),&ubSize);
      /* set to the start of the string */
      ++(*pPtr);

      sTrack_t *pTrack = pSeq->arTracks[trackNb];

      pTrack->pTrackName = (uint8 *)gUserMemAlloc(128*sizeof(uint8),PREFER_TT,0);

      if(pTrack->pTrackName!=NULL)
      {
        amMemSet(pTrack->pTrackName, 0, 128);
        strncpy((char *)pTrack->pTrackName,(char *)(*pPtr), 127);
        pTrack->pTrackName[ubLenght]='\0';
        //amMemCpy(pTrack->pTrackName, (*pPtr),ubLenght*sizeof(uint8));
      }

        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace("meta size: %d ",ubLenght);
        amTrace("%s "NL,pTrack->pTrackName);
#endif
      ret = AM_OK;
    } break;
    case MT_INSTRNAME:
    {
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Instrument name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace("meta size: %d",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(uint8) );
        (*pPtr)=((*pPtr)+ubLenght);

#ifdef MIDI_PARSER_DEBUG
        amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;
    case MT_LYRICS:
    {
#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %u\tMeta event: Lyrics: ",(unsigned long)delta);
#endif
      (*pPtr)++;
      ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
      amTrace("meta size: %d ",ubLenght);
#endif
      /* set to the start of the string */
      (*pPtr)++;
      amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(uint8));
      (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
      amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;

    case MT_MARKER:
    {
#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %u\tMeta event: Marker: ", delta);
#endif
      sMarker_EventBlock_t *pEvntBlock=0;

      (*pPtr)++;
      ubLenght=readVLQ((*pPtr),&ubSize);

    /* set to the start of the string */
      (*pPtr)++;
      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_META_MARKER;

      getEventCallbackHandler(T_META_MARKER,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr=(void *)tempBuf;

      pEvntBlock=(sMarker_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->pMarkerName = (uint8 *)gUserMemAlloc(ubLenght+1,PREFER_TT,0);
      amMemSet(pEvntBlock->pMarkerName,0,((ubLenght+1)*sizeof(uint8)));
      amMemCpy(pEvntBlock->pMarkerName,(*pPtr),ubLenght*sizeof(uint8));
      pEvntBlock->pMarkerName[ubLenght]='\0';
      (*pPtr)=((*pPtr)+ubLenght);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

#ifdef MIDI_PARSER_DEBUG
      amTrace("meta size: %d ",ubLenght);
      amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;
    
    case MT_CUEPOINT:
    {
#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %u\tMeta event: Cuepoint"NL,(unsigned long)delta);
#endif
    sCuePoint_EventBlock_t *pEvntBlock=0;

    (*pPtr)++;
    ubLenght=readVLQ((*pPtr),&ubSize);
    /* set to the start of the string */
    (*pPtr)++;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_META_CUEPOINT;

    getEventCallbackHandler(T_META_CUEPOINT,&tempEvent.eventCb);
    uint8 tempBuf[tempEvent.eventCb.size];

    tempEvent.dataPtr=(void *)tempBuf;

    pEvntBlock=(sCuePoint_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->pCuePointName=0;

    pEvntBlock->pCuePointName = (uint8 *)gUserMemAlloc(ubLenght+1,PREFER_TT,0);
    amMemSet(pEvntBlock->pCuePointName,0,((ubLenght+1)*sizeof(uint8)));
    amMemCpy(pEvntBlock->pCuePointName,(*pPtr),ubLenght*sizeof(uint8));
    pEvntBlock->pCuePointName[ubLenght]='\0';

    (*pPtr)=((*pPtr)+ubLenght);

    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
    addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
    addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

#ifdef MIDI_PARSER_DEBUG
    amTrace("meta size: %d ",ubLenght);
    amTrace("%s "NL,pEvntBlock->pCuePointName);
#endif
      ret = AM_OK;
    } break;

    case MT_PROGRAM_NAME:
    {
        /* program(patch) name */
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Program (patch) name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(uint8) );
        (*pPtr)=((*pPtr)+ubLenght);

#ifdef MIDI_PARSER_DEBUG
        amTrace("meta size: %d ",ubLenght);
        amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;
    case MT_DEVICE_NAME:
    {
      /* device (port) name */
#ifdef MIDI_PARSER_DEBUG
    amTrace("delta: %u\tMeta event: Device (port) name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(uint8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace("meta size: %d ",ubLenght);
        amTrace("%s "NL,textBuffer);
#endif
      ret = AM_OK;
    } break;
    case MT_CH_PREFIX:{
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Channel prefix"NL,(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((uint32)(*pPtr))+ubLenght*sizeof(uint8);
        *pPtr=(uint8*)addr;
      return AM_OK;
    } break;
    case MT_MIDI_CH:{ /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Midi channel nb: %d"NL,(unsigned long)delta,*(*pPtr));
#endif
        (*pPtr)++;
      ret = AM_OK;
    }break;
    case MT_MIDI_PORT:{ /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
#ifdef MIDI_PARSER_DEBUG
        amTrace("delta: %u\tMeta event: Midi port nb: %d"NL,(unsigned long)delta,*(*pPtr));
#endif
        (*pPtr)++;
      ret = AM_OK;
    } break;
    case MT_EOT:
    {
#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %u\tMeta event: End of track"NL,(unsigned long)delta);
#endif
    sEot_EventBlock_t *pEvntBlock=0;
    (*pPtr)++;
    ubLenght=(*(*pPtr));
    (*pPtr)++;
    addr=((uint32)(*pPtr))+ubLenght*sizeof(uint8);
    *pPtr=(uint8*)addr;

    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_META_EOT;

    getEventCallbackHandler(T_META_EOT,&tempEvent.eventCb);
    uint8 tempBuf[tempEvent.eventCb.size];

    tempEvent.dataPtr=(void *)tempBuf;

    pEvntBlock=(sEot_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->dummy=0L;   //dummy value

    *bEOT=TRUE;
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
  ret = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
   ret = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
    } break;
    case MT_SET_TEMPO:
    {
     /* sets tempo in track, should be in the first track, if not 120 BPM is assumed */
      uint32 val1,val2,val3;
      uint8 ulVal[3]={0};   /* for retrieving set tempo info */
      sTempo_EventBlock_t *pEvntBlock=NULL;
    #ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %u\tMeta event: Set tempo: ",(unsigned long)delta);
    #endif

      (*pPtr)++;
      ubLenght=(*(*pPtr));
      (*pPtr)++;
      /* get those 3 bytes */
      amMemCpy(ulVal, (*pPtr),ubLenght*sizeof(uint8) );
      addr=((uint32)(*pPtr))+ubLenght*sizeof(uint8);
      *pPtr=(uint8*)addr;

      val1=ulVal[0],val2=ulVal[1],val3=ulVal[2];
      val1=(val1<<16)&0x00FF0000L;
      val2=(val2<<8)&0x0000FF00L;
      val3=(val3)&0x000000FFL;

      /* range: 0-8355711 ms, 24 bit value */
      val1=val1|val2|val3;

      tempEvent.uiDeltaTime=delta;
      tempEvent.type=T_META_SET_TEMPO;

      getEventCallbackHandler(T_META_SET_TEMPO,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr=(void *)tempBuf;

      pEvntBlock=(sTempo_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->eventData.tempoVal= val1;

#ifdef MIDI_PARSER_DEBUG
      amTrace("%lu ms per quarter-note"NL, val1);
#endif
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      ret = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      ret = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif
    } break;
    case MT_SMPTE_OFFSET:
    {
       sSMPTEoffset SMPTEinfo;
#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %lu\tMeta event: SMPTE offset:"NL,delta);
#endif
      (*pPtr)++;
      ubLenght=(*(*pPtr));
      (*pPtr)++;
      amMemCpy (&SMPTEinfo,(*pPtr),sizeof(sSMPTEoffset));

      addr=((uint32)(*pPtr))+ubLenght*sizeof(uint8);
      *pPtr=(uint8*)addr;
/* print out info */
#ifdef MIDI_PARSER_DEBUG
      amTrace("hr: %d"NL,SMPTEinfo.hr);
      amTrace("mn: %d"NL,SMPTEinfo.mn);
      amTrace("se: %d"NL,SMPTEinfo.fr);
      amTrace("fr: %d"NL,SMPTEinfo.fr);
      amTrace("ff: %d"NL,SMPTEinfo.ff);
#endif
      ret = AM_OK;
    } break;
    case MT_TIME_SIG:
    {
     sTimeSignature timeSign;
     sTimeSignature_EventBlock_t *pEvntBlock=NULL;

#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %lu\tMeta event: Time signature: ",delta);
#endif
      (*pPtr)++;
      ubLenght=(*(*pPtr));
      (*pPtr)++;

      amMemCpy(&timeSign,(*pPtr),sizeof(sTimeSignature));
      addr=((uint32)(*pPtr))+ubLenght*sizeof(uint8);
      *pPtr=(uint8*)addr;

      tempEvent.uiDeltaTime=delta;
      tempEvent.type = T_META_SET_SIGNATURE;

      getEventCallbackHandler(T_META_SET_SIGNATURE,&tempEvent.eventCb);
      uint8 tempBuf[tempEvent.eventCb.size];

      tempEvent.dataPtr=(void *)tempBuf;

      pEvntBlock=(sTimeSignature_EventBlock_t *)tempEvent.dataPtr;
      pEvntBlock->timeSignature=timeSign;

      /* print out info */
#ifdef MIDI_PARSER_DEBUG
      amTrace("Time signature change nn: %d\tdd: %d\tcc: %d\tbb: %d\r"NL,timeSign.nn,timeSign.dd,timeSign.cc,timeSign.bb);
#endif
    /* add event to list */
#ifdef EVENT_LINEAR_BUFFER
      ret = addEvent(pSeq, &pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#else
      ret = addEvent(&pSeq->arTracks[trackNb]->pTrkEventList, &tempEvent );
#endif

    } break;
    case MT_KEY_SIG:
    {
      uint8 param1=0,param2=0;

#ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %lu\tMeta event: Key signature: ",delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        param1=(*(*pPtr));
        (*pPtr)++;
        param2=(*(*pPtr));
#ifdef MIDI_PARSER_DEBUG
        if (param2==0) {amTrace("scale: major ",0);}
        else if(param2==1) {amTrace("scale: minor ",0);}
         else { amTrace("error: wrong key signature scale. ",0); }

        if(param1==0)
            {amTrace("Key of C"NL,0);}
        else if (((int8)param1==-1))
            {amTrace("1 flat"NL,0);}
        else if(((int8)param1)==1)
            {amTrace("1 sharp"NL,0);}
        else if ((param1>1&&param1<=7))
            {amTrace(" %d sharps"NL,param1);}
        else if (( ((int8)param1)<-1&& ((int8)param1)>=-7))
            {amTrace(" %ld flats"NL,(uint32)param1);}
        else {amTrace(" error: wrong key signature. %d"NL,param1);}
#endif

      (*pPtr)++;
      ret = AM_OK;
    } break;
    case MT_SEQ_SPEC:
    {
    #ifdef MIDI_PARSER_DEBUG
      amTrace("delta: %lu\tMeta event: Sequencer specific data."NL,delta);
    #endif
      (*pPtr)++;
      ubLenght=(*(*pPtr));
      (*pPtr)++;
      addr=((uint32)(*pPtr))+ubLenght * sizeof(uint8);
      *pPtr=(uint8*)addr;
      ret = AM_OK;
    } break;
    default:
    {
    #ifdef MIDI_PARSER_DEBUG
       amTrace("delta: %lu\tUnknown meta event."NL,delta);
    #endif
       (*pPtr)++;
       ubLenght=(*(*pPtr));
       /* we should put here assertion failed or something with "send this file to author" message */
       /* file also could be broken */

    #ifdef MIDI_PARSER_DEBUG
        amTrace("id: %d, size: %d"NL /*parameters: %ld "NL*/,ubVal,*(*pPtr));
    #endif
        (*pPtr)=(*pPtr)+ubLenght;
        ret = AM_OK;
    } break;
 };

  return ret;
}

/** processes the MIDI 0,1,2 track events
* @param pSeq pointer to AMIDI sequence structure
* @param startPtr pointer to an address containing start of Track MIDI chunk
* @param endAddr address, where track data ends
* @param trackNb track number to process (from 0 to 15)
* @param iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
* @return pointer to the next track or NULL if EOT occured.
*/

/* handles the events in tracks and returns pointer to the next midi track */
static INLINE void *processMidiTrackEvents(sSequence_t *pSeq, void**startPtr, const void *endAddr, const uint8 trackNb, int16 *iRetVal )
{
  uint8 usSwitch=0;
  uint16 recallStatus=0;

  Bool bEOF=FALSE;

  /* execute as long we are on the end of file or EOT meta occured,
    50% midi track headers is broken, so the web says ;)) */

  uint8 *pCmd=((uint8 *)(*startPtr));
  uint8 ubSize=0;

  while ( ((pCmd!=endAddr) && (bEOF!=TRUE) && ((*iRetVal) == AM_OK)) )
  {
    uint32 delta=0L;
    
    /* read delta time, pCmd should point to the command data */
    delta=readVLQ(pCmd,&ubSize);

    pCmd=(uint8 *)((uint32)pCmd+ubSize*sizeof(uint8));

    /* handling of running status */
    /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
    /* else set recallStatus = 0 and do nothing special */
    ubSize=(*pCmd);

    if( (!(amIsMidiChannelEvent(ubSize))&&(recallStatus==1)&&(!(amIsMidiRtCmdOrSysex(ubSize)))))
    {
      /*recall last cmd byte */
      usSwitch=g_runningStatus;
      usSwitch=(usSwitch&0xF0);
    }
    else
    {
      /* check if the new cmd is the system one*/
      recallStatus=0;

      if(amIsMidiRtCmdOrSysex(ubSize))
      {
        usSwitch=ubSize;
      }
      else
      {
        usSwitch= ubSize & 0xF0;
      }
    }

    /* decode event and write it to our custom structure */
    switch(usSwitch)
    {
      case EV_NOTE_OFF:
        *iRetVal=amNoteOff(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_NOTE_ON:
        *iRetVal=amNoteOn(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_NOTE_AFTERTOUCH:
        *iRetVal=amNoteAft(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_CONTROLLER:
        *iRetVal=amController(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_PROGRAM_CHANGE:
        *iRetVal=amProgramChange(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_CHANNEL_AFTERTOUCH:
        *iRetVal=amChannelAft(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_PITCH_BEND:
        *iRetVal=amPitchBend(pSeq,&pCmd,&recallStatus, delta, trackNb );
      break;
      case EV_META:
        *iRetVal=amMetaEvent(pSeq,&pCmd, delta, trackNb,&bEOF);
      break;
      case EV_SOX:                          	/* SySEX midi exclusive */
        recallStatus=0; 	                /* cancel out midi running status */
        *iRetVal=amSysexMsg(pSeq,&pCmd,delta, trackNb);
      break;
      case SC_MTCQF:
        recallStatus=0;                        /* Midi time code quarter frame, 1 byte */
        amTrace("Event: System common MIDI time code qt frame"NL,0);
        ++pCmd;
        ++pCmd;
      break;
      case SC_SONG_POS_PTR:
        amTrace("Event: System common Song position pointer"NL,0);
        recallStatus=0;                      /* Song position pointer, 2 data bytes */
        ++pCmd;
        ++pCmd;
        ++pCmd;
      break;
      case SC_SONG_SELECT:              /* Song select 0-127, 1 data byte*/
        amTrace("Event: System common Song select"NL,0);
        recallStatus=0;
        ++pCmd;
        ++pCmd;
      break;
      case SC_UNDEF1:                   /* undefined */
      case SC_UNDEF2:                  /* undefined */
    amTrace("Event: System common not defined."NL,0);
        recallStatus=0;
        ++pCmd;
      break;
      case SC_TUNE_REQUEST:             /* tune request, no data bytes */
    amTrace("Event: System tune request."NL,0);
        recallStatus=0;
        ++pCmd;
      break;
      default:
      {
        amTrace("Event: Unknown type: %d"NL,(*pCmd));
        /* unknown event, do nothing or maybe throw error? */
      } break;
    }
} /*end of decode events loop */

  /* return the next track data */
  return(pCmd);
}


/* at this point pCurSequence should have the info about the type of file that resides in memory,
because we have to know if we have to dump event data to one eventlist or several ones */
/* all the events found in the track will be dumped to the sSequenceState_t structure  */

void *processMidiTracks(void *trackStartPtr, const eMidiFileType fileTypeFlag, sSequence_t **ppCurSequence, int16 *iRetVal )
{

sChunkHeader *pHeader = (sChunkHeader *)trackStartPtr;

if(pHeader->id != ID_MTRK) 
{
  amTrace("Fatal error: Wrong midi track id"NL,0);
  return NULL;
}

uint32 ulChunkSize = pHeader->headLenght;

trackStartPtr = (void *)((uint32)trackStartPtr + sizeof(sChunkHeader)); // get events start
void *endPtr = ((void *)((uint32)trackStartPtr + ulChunkSize));

sSequence_t * const sequence = *ppCurSequence;

if(sequence==0) 
{
  amTrace("Fatal error: Sequence pointer is null!"NL,0);
  return NULL;
}

const uint8 numTracks = sequence->ubNumTracks;

amTrace("Number of tracks to process: %d"NL, numTracks);

uint8 currentTrackNb = 0;

switch(fileTypeFlag)
{
    case T_MIDI0:
    {
      /* we have only one track data to process */
      /* add all of them to given track */

      sequence->seqType = ST_SINGLE;

      sTrack_t *pTempTrack = sequence->arTracks[currentTrackNb];
      pTempTrack->currentState.playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
      pTempTrack->currentState.currentTempo = DEFAULT_MPQN;
      pTempTrack->currentState.currentBPM = DEFAULT_BPM;

      trackStartPtr = processMidiTrackEvents(sequence, &trackStartPtr, endPtr, currentTrackNb, iRetVal );

      if((*iRetVal) == AM_ERR) 
      {
        return NULL;
      }

    } break;

    case T_MIDI1:
    {
      sequence->seqType = ST_MULTI;

      while(((pHeader!=0) && (pHeader->id==ID_MTRK) && (currentTrackNb<numTracks)))
      {
        /* we have got track data :)) */
        /* add all of them to given track */
        sTrack_t *pTempTrack = sequence->arTracks[currentTrackNb];
        pTempTrack->currentState.playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
        pTempTrack->currentState.currentTempo = DEFAULT_MPQN;
        pTempTrack->currentState.currentBPM = DEFAULT_BPM;

        trackStartPtr = processMidiTrackEvents(sequence, &trackStartPtr, endPtr, currentTrackNb, iRetVal );

        if((*iRetVal) == AM_ERR) 
        {
          return NULL;
        }

        /* increase current track counter */
        ++currentTrackNb;

        //prevent reading chunk after processing the last track
        if(currentTrackNb < numTracks)
        {
          /* get next data chunk info */
          pHeader=(sChunkHeader *)trackStartPtr;
          ulChunkSize=pHeader->headLenght;

          /* omit Track header */
          trackStartPtr = (void *)((uint32)trackStartPtr + sizeof(sChunkHeader));
          endPtr = ((void *)((uint32)trackStartPtr + ulChunkSize));
        } 
        else
        {
            pHeader=0;
        }
     
      }

     } break;
    
    case T_MIDI2:
    {
      /* handle MIDI 2, multitrack type */
      /* create several track lists according to numTracks */
      sequence->seqType = ST_MULTI_SUB;

      /* tracks inited, now insert track data */
      while(((pHeader!=0)&&(pHeader->id==ID_MTRK)&&(currentTrackNb<numTracks)))
      {
        /* we have got track data :)), add all of them to given track */
        sTrack_t *pTempTrack = sequence->arTracks[currentTrackNb];
        pTempTrack->currentState.playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
        pTempTrack->currentState.currentTempo = DEFAULT_MPQN;
        pTempTrack->currentState.currentBPM = DEFAULT_BPM;
    
        trackStartPtr = processMidiTrackEvents(sequence, &trackStartPtr, endPtr, currentTrackNb, iRetVal );

        if((*iRetVal) == AM_ERR) 
        {
          return NULL;
        }

        /* increase track counter */
        ++currentTrackNb;

        if(currentTrackNb < numTracks)
        {
          /* get next data chunk info */
          pHeader = (sChunkHeader *)trackStartPtr;
          ulChunkSize = pHeader->headLenght;

          /* omit Track header */
          trackStartPtr = (void *)((uint32)trackStartPtr + sizeof(sChunkHeader));
          endPtr = ((void *)((uint32)trackStartPtr + ulChunkSize));
        }
        else
        {
            pHeader=0;
        }

      }
    } break;
    default:
    {
      AssertMsg(0,"Fatal: Unhandled midi file type.");
    } break;
 };
 
 amTrace("Finished processing..."NL,0);
 return NULL;
}

/* combine bytes function for pitch bend */
uint16 amCombinePitchBendBytes(const uint8 bFirst, const uint8 bSecond)
{
    uint16 val;
    val = (uint16)bSecond;
    val<<=7;
    val|=(uint16)bFirst;
 return(val);
}
