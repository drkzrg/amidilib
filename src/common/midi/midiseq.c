/**
    sending midi events to external midi module
*/

/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "midiseq.h"
#include "amidiseq.h"
#include "midi_cmd.h"
#include "midi_rep.h"
#include "timing/miditim.h"


/* string table with all event names */
const uint8 *arEventNames[T_EVT_COUNT]={
    "Note On",
    "Note Off",
    "Note Aftertouch",
    "Controller",
    "Program Change",
    "Channel Aftertouch",
    "Pitch bend",
    "Set tempo(Meta)",
    "End of track(Meta)",
    "Cuepoint(Meta)",
    "Marker(Meta)",
    "Set time signature (Meta)",
    "SysEX Msg"
};

/*returns pointer to NULL terminated string with event name */
/* id is enumerated value from eEventType */
const uint8 *getEventName(const uint32 id)
{
  if(id<T_EVT_COUNT)
    return ((const uint8 *)arEventNames[id]);
  else return NULL;
}

/****************** event function prototypes */
//common
static void fSetTempo(const void *pEvent)
{
 sTempo_EventBlock_t *pPtr=(sTempo_EventBlock_t *)pEvent;
 sSequence_t *seq = getActiveAmSequence();

 // set new tempo value and indicate that tempo has changed
 // it will be handled in interrupt routine
 const uint8 activeTrack = seq->ubActiveTrack;
 sTrackState_t *pCurTrackState = &(seq->arTracks[activeTrack]->currentState);

 pCurTrackState->currentTempo = pPtr->eventData.tempoVal;
 amTrace("fSetTempo %u\n",pCurTrackState->currentTempo);
 pCurTrackState->currentBPM = 60000000/pCurTrackState->currentTempo;
 seq->timeStep = amCalculateTimeStep(pCurTrackState->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);
}

static void fHandleEOT(const void *pEvent){
 sEot_EventBlock_t *pPtr=(sEot_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
 amTrace((const uint8*)"End of track...\n");
#endif
}

static void fHandleCuePoint(const void *pEvent){
 sCuePoint_EventBlock_t *pPtr=(sCuePoint_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
 amTrace((const uint8*)"Cuepoint %s.\n",pPtr->pCuePointName);
#endif

}

static void fHandleMarker(const void *pEvent){
 sMarker_EventBlock_t *pPtr=(sMarker_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
 amTrace((const uint8*)"Marker %s.\n",pPtr->pMarkerName);
#endif
}

static void fHandleSignatureChange(const void *pEvent){
  sTimeSignature_EventBlock_t *pPtr=(sTimeSignature_EventBlock_t *)pEvent;

#ifdef DEBUG_BUILD
 amTrace((const uint8*)"Time Signature change nn: %d, dd: %d, cc: %d, bb %d.\r\n",pPtr->timeSignature.nn,pPtr->timeSignature.dd,pPtr->timeSignature.cc,pPtr->timeSignature.bb);
#endif

}

uint8 getChannelNbFromEventBlock(const sEventBlock_t *pBlock)
{
  switch(pBlock->type)
  {
    case T_NOTEON:
      return ((sNoteOn_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    case T_NOTEOFF:
      return ((sNoteOff_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    case T_NOTEAFT:
      return ((sNoteAft_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    case T_CONTROL:
      return ((sController_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    case T_PRG_CH:
      return ((sPrgChng_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    case T_CHAN_AFT:
      return ((sChannelAft_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    case T_PITCH_BEND:
      return ((sPitchBend_EventBlock_t *)pBlock->dataPtr)->ubChannelNb;
    break;
    //these events doesn't contain any delta information
    //so we are returning channel out of range	     
    case T_META_SET_TEMPO:
    case T_META_EOT:
    case T_META_CUEPOINT:
    case T_META_MARKER:
    case T_META_SET_SIGNATURE:
    case T_SYSEX:
      return 127;
    break;
  };
 return 127;
}


/* event id is mapped to the position in the array, functionPtr, parameters struct */
#ifdef IKBD_MIDI_SEND_DIRECT
//event handlers which copy event data to output buffer
static void  fNoteOnCopyData(const void *pEvent){
    sNoteOn_EventBlock_t *pPtr=(sNoteOn_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Note On data to buffer ch: %d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
#endif
    copy_note_on(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

static void  fNoteOffCopyData(const void *pEvent){
    sNoteOff_EventBlock_t *pPtr=(sNoteOff_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Note Off data to buffer ch: %d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
#endif
    copy_note_off(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

static void  fNoteAftCopyData(const void *pEvent){
    sNoteAft_EventBlock_t *pPtr=(sNoteAft_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Note Aftertouch data to buffer ch:%d note:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
#endif

    copy_polyphonic_key_press(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
}

 static void  fProgramChangeCopyData (const void *pEvent){
    sPrgChng_EventBlock_t *pPtr=(sPrgChng_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Program change data to buffer ch:%d pn:%d...\n",pPtr->ubChannelNb,pPtr->eventData.programNb);
#endif
      //TODO: set program number depending on operating mode
      //note: operating mode is dependent from midi file content too. Set mode based file data with option of global override (user configurable)?
    copy_program_change(pPtr->ubChannelNb,pPtr->eventData.programNb);
}

 static void  fControllerCopyData(const void *pEvent){
    sController_EventBlock_t *pPtr=(sController_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Controller data to buffer ch:%d controller:%d value:%d...\n",pPtr->ubChannelNb,pPtr->eventData.controllerNb,pPtr->eventData.value);
#endif
    copy_control_change(pPtr->eventData.controllerNb, pPtr->ubChannelNb, pPtr->eventData.value,0x00);
}

static void  fChannelAftCopyData(const void *pEvent){
    sChannelAft_EventBlock_t *pPtr=(sChannelAft_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Channel Aftertouch data to buffer ch:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.pressure);
#endif
    copy_channel_pressure(pPtr->ubChannelNb,pPtr->eventData.pressure);
}

static void fPitchBendCopyData(const void *pEvent){
    sPitchBend_EventBlock_t *pPtr=(sPitchBend_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Copying Pitch bend data to buffer ch:%d LSB:%d MSB:%d...\n",pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
#endif
    copy_pitch_bend_2(pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
}

static void fHandleSysEXCopyData(const void *pEvent){
  sSysEX_EventBlock_t *pPtr=(sSysEX_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
  amTrace((const uint8*)"Copy SysEX Message.\n");
#endif
//TODO: check buffer overflow
   amMemCpy(&MIDIsendBuffer[MIDIbytesToSend],pPtr->pBuffer,pPtr->bufferSize);
   MIDIbytesToSend+=pPtr->bufferSize;
}

/** !ORDER IS IMPORTANT! and has to be the same as in enums with T_EVT_COUNT. Additionally
the ordering of members should be the same as described in type sEventList. */

////////////////////////////////////////////////////////////////////////////////////////
//callbacks for copying events to intermediate buffer which is sent in one go
static const sEventInfoBlock_t arSequenceCmdCallbackTable[T_EVT_COUNT] = 
{
   {sizeof(sNoteOn_EventBlock_t),fNoteOnCopyData},
   {sizeof(sNoteOff_EventBlock_t),fNoteOffCopyData},
   {sizeof(sNoteAft_EventBlock_t), fNoteAftCopyData},
   {sizeof(sController_EventBlock_t),fControllerCopyData},
   {sizeof(sPrgChng_EventBlock_t),fProgramChangeCopyData},
   {sizeof(sChannelAft_EventBlock_t),fChannelAftCopyData},
   {sizeof(sPitchBend_EventBlock_t),fPitchBendCopyData},
   {sizeof(sTempo_EventBlock_t),fSetTempo},
   {sizeof(sEot_EventBlock_t),fHandleEOT},
   {sizeof(sCuePoint_EventBlock_t),fHandleCuePoint},
   {sizeof(sMarker_EventBlock_t),fHandleMarker},
   {sizeof(sTimeSignature_EventBlock_t),fHandleSignatureChange},
   {sizeof(sSysEX_EventBlock_t),fHandleSysEXCopyData}
};

void getEventCallbackHandler(const uint8 eventType, sEventInfoBlock_t *infoBlk)
{
    infoBlk->size = arSequenceCmdCallbackTable[eventType].size;
    infoBlk->func = arSequenceCmdCallbackTable[eventType].func;
}

#else

static void  fNoteOn(const void *pEvent) {
    sNoteOn_EventBlock_t *pPtr=(sNoteOn_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Note On data to midi out ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
#endif
    note_on(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

 static void  fNoteOff(const void *pEvent){
    sNoteOff_EventBlock_t *pPtr=(sNoteOff_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Note Off data to midi out ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
#endif
    note_off(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

 static void  fNoteAft(const void *pEvent){
    sNoteAft_EventBlock_t *pPtr=(sNoteAft_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Note Aftertouch data to midi out ch:%d note:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
#endif
    polyphonic_key_press(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
}

 static void  fProgramChange (const void *pEvent){
    sPrgChng_EventBlock_t *pPtr=(sPrgChng_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Program change data to midi out ch:%d pn:%d...\n",pPtr->ubChannelNb,pPtr->eventData.programNb);
#endif
    program_change(pPtr->ubChannelNb,pPtr->eventData.programNb);
}

 static void  fController(const void *pEvent){
    sController_EventBlock_t *pPtr=(sController_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Controller data to midi out ch:%d controller:%d value:%d...\n",pPtr->ubChannelNb,pPtr->eventData.controllerNb,pPtr->eventData.value);
#endif
    control_change(pPtr->eventData.controllerNb, pPtr->ubChannelNb, pPtr->eventData.value,0x00);
}

 static void  fChannelAft(const void *pEvent){
    sChannelAft_EventBlock_t *pPtr=(sChannelAft_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Channel Aftertouch data to midi out ch:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.pressure);
#endif
    channel_pressure (pPtr->ubChannelNb,pPtr->eventData.pressure);
}

 static void  fPitchBend(const void *pEvent){
    sPitchBend_EventBlock_t *pPtr=(sPitchBend_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
    amTrace((const uint8*)"Sending Pitch bend data to midi out ch:%d LSB:%d MSB:%d...\n",pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
#endif
    pitch_bend_2 (pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
}

static void fHandleSysEX(const void *pEvent){
  sSysEX_EventBlock_t *pPtr=(sSysEX_EventBlock_t *)pEvent;
#ifdef DEBUG_BUILD
  amTrace((const uint8*)"SysEX Message.\n");
#endif

}

/** !ORDER IS IMPORTANT! and has to be the same as in enums with T_EVT_COUNT. Additionally
the ordering of members should be the same as described in type sEventList. */

//callbacks for sending directly events to external device through midi out port
static const sEventInfoBlock_t arSequenceCmdCallbackTable[T_EVT_COUNT] = 
{
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
   {sizeof(sTimeSignature_EventBlock_t),fHandleSignatureChange},
   {sizeof(sSysEX_EventBlock_t),fHandleSysEX}
};

void getEventCallbackHandler(const uint8 eventType, sEventInfoBlock_t *infoBlk)
{
    infoBlk->size = arSequenceCmdCallbackTable[eventType].size;
    infoBlk->func = arSequenceCmdCallbackTable[eventType].func;
}

#endif
