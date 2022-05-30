
// MIDI replay core

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "config.h"
#include "amidilib.h"
#include "timing/miditim.h"
#include "timing/mfp.h"
#include "midi_send.h"
#include "containers/list.h"

#include "core/amprintf.h"

static sSequence_t *g_CurrentSequence=0;

sSequence_t* const getActiveAmSequence(void)
{
  AssertMsg(g_CurrentSequence=!0,"Fatal error: current sequence is NULL");
  return g_CurrentSequence;
}


void initAmSequence(sSequence_t *seq, const eTimerType timerType)
{
 
  AssertMsg(seq!=0,"Passed sequence cannot be NULL.");

  g_CurrentSequence=0;

  sTrack_t *pTrack;
  sTrackState_t *pTrackState;
  uint8 activeTrack=seq->ubActiveTrack;

  g_CurrentSequence = seq;

  Supexec(clearMidiOutputBuffer);

  for(uint16 i=0;i<seq->ubNumTracks;++i)
  {
    pTrack = seq->arTracks[i];

    if(pTrack)
    {
      pTrackState=&(pTrack->currentState);
      pTrackState->currentTempo=DEFAULT_MPQN;
      pTrackState->currentBPM=DEFAULT_BPM;
      pTrackState->timeElapsedInt=0L;
      pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
      pTrackState->playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
    }
  } 
  
  seq->timeElapsedFrac=0L;
  seq->timeStep=0L;
  pTrackState=&(seq->arTracks[activeTrack]->currentState);
  seq->timeStep=amCalculateTimeStep(pTrackState->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
  Supexec(flushMidiSendBuffer);
#endif

  uint8 mode=0,data=0;
  
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

#ifdef DEBUG_BUILD
  amTrace("%dhz update interval, Time step: %d\r"NL,SEQUENCER_UPDATE_HZ,seq->timeStep);
  amTrace("calculated mode: %d, data: %d"NL,mode,data);
#endif
    
  if(seq->seqType==ST_SINGLE)
  {
    installReplayRout(mode, data, FALSE, timerType);
  }
  else if(seq->seqType==ST_MULTI)
  {
    installReplayRout(mode, data, TRUE, timerType);
  }
  else
  {
    AssertMsg(0,"initSeq(): Unsupported sequence type. Couldn't install replay routine.");
  }

  
 return;
}

void initAmSequenceManual(sSequence_t *seq)
{
  AssertMsg(seq!=0,"Passed sequence cannot be NULL.");

 	sTrackState_t *pTrackState=0;

  g_CurrentSequence=0;
  g_CurrentSequence=seq;

  for(uint16 i=0;i<seq->ubNumTracks;++i)
  {
    sTrack_t *pTrack = seq->arTracks[i];

    if(pTrack)
    {
      pTrackState=&(pTrack->currentState);
      pTrackState->currentTempo = DEFAULT_MPQN;
      pTrackState->currentBPM = DEFAULT_BPM;
      pTrackState->timeElapsedInt = 0;
      pTrackState->currEventPtr = pTrack->pTrkEventList;               // set begining of event list
      pTrackState->playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));
    }
  } 
  
  seq->timeElapsedFrac=0L;
  seq->timeStep=0L;
  seq->timeStep = amCalculateTimeStep(seq->arTracks[0]->currentState.currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
  Supexec(flushMidiSendBuffer);
#endif

 return;
}

void onEndSequence(void)
{

sTrack_t *pTrack=0;
AssertMsg(g_CurrentSequence!=0,"Current sequence is NULL.");

uint8 activeTrack=0;
activeTrack=g_CurrentSequence->ubActiveTrack;
sTrackState_t *pTrackState=&(g_CurrentSequence->arTracks[activeTrack]->currentState);

if(pTrackState->playState&TM_PLAY_ONCE)
{
  //reset set state to stopped
  //reset song position on all tracks
  pTrackState->playState&=(~(TS_PS_PLAYING|TS_PS_PAUSED));
}
else
{ 
  // loop
  pTrackState->playState&=(~TS_PS_PAUSED);
  pTrackState->playState|=TS_PS_PLAYING;
}

// reset all controllers
amAllNotesOff(16);

for (uint16 i=0;i<g_CurrentSequence->ubNumTracks;++i)
{
  pTrack=g_CurrentSequence->arTracks[i];

  if(pTrack)
  {
    pTrack->currentState.timeElapsedInt=0L;
    pTrack->currentState.currentTempo=DEFAULT_MPQN;
    pTrack->currentState.currentBPM=DEFAULT_BPM;
    pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
  }
}

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif
  // reset all tracks state
  g_CurrentSequence->timeElapsedFrac = 0L;
  g_CurrentSequence->timeStep = amCalculateTimeStep(pTrackState->currentBPM, g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);
}

volatile static Bool bEventSent=FALSE;
volatile static Bool bEOTflag=FALSE;
volatile static Bool bStopped=FALSE;
volatile static Bool endOfSequence=FALSE;
volatile static uint32 TimeAdd=0;
volatile static evntFuncPtr myFunc=NULL;
volatile static sEventList *pCurrentEvent=0;
volatile static uint32 timeElapsed=0;
volatile static sTrackState_t *pActiveTrackState=0;
volatile static sTrack_t *pTrack=0;

// single track handler
void updateStepSingle(void)
{
  if(g_CurrentSequence==0) return;
 
  bStopped = FALSE;

 //get track, there is only one active
  pTrack = g_CurrentSequence->arTracks[0];
 
  AssertMsg(pTrack!=0,"Pointer to first track in sequence is null");
      
  pActiveTrackState=&(pTrack->currentState);

  const uint16 currentPlayState = pActiveTrackState->playState;

  // check sequence state if paused do nothing
  if(currentPlayState & TS_PS_PAUSED) 
  {
    amAllNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
    flushMidiSendBuffer();
#endif
    return;
  }
 
  if(currentPlayState & TS_PS_PLAYING)
  {
     bStopped = FALSE; //we replaying, so we have to reset this flag
  
     pCurrentEvent = pActiveTrackState->currEventPtr;
     volatile sEventBlock_t *evntBlock= &pCurrentEvent->eventBlock;
             
     bEOTflag = (Bool)(evntBlock->type == T_META_EOT);
     timeElapsed = pActiveTrackState->timeElapsedInt;

      // reset
      bEventSent=FALSE;
      
      if( ((bEOTflag != TRUE) && (evntBlock->uiDeltaTime == timeElapsed)) )
      {
        endOfSequence = FALSE;

        myFunc = evntBlock->eventCb.func;
        (*myFunc)((void *)evntBlock->dataPtr);

        //go to next event
        pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pNext;
        pCurrentEvent = pActiveTrackState->currEventPtr;
        
        //check end of track
        if(pCurrentEvent!=0)
        {
          evntBlock = &pCurrentEvent->eventBlock;
          bEOTflag = (Bool)(evntBlock->type == T_META_EOT);

          //check if next events are null and pack buffer until first next non zero delta
          while( pCurrentEvent && ((bEOTflag==TRUE) && (evntBlock->uiDeltaTime==0)) )
          {
            //handle event
            //execute callback which copies data to midi buffer (_MIDIsendBuffer)
            myFunc = evntBlock->eventCb.func;
            (*myFunc)((void *)evntBlock->dataPtr);

            // go to next event
            pActiveTrackState->currEventPtr = pActiveTrackState->currEventPtr->pNext;
            pCurrentEvent=pActiveTrackState->currEventPtr;
      
            if(pCurrentEvent)
            {
              evntBlock = &pCurrentEvent->eventBlock;
              bEOTflag = (Bool)(pCurrentEvent->eventBlock.type == T_META_EOT);
            }
        }

        if(bEOTflag == TRUE)
        {
          endOfSequence = TRUE;
        }

       bEventSent = TRUE;
      }
    } //endif

    g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
    TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
    g_CurrentSequence->timeElapsedFrac &= 0xffff;

    if(TimeAdd>1)
        TimeAdd=1;

   //add time elapsed
   if(bEventSent==TRUE)
   {
     pActiveTrackState->timeElapsedInt=0;
   }
   else
   {
     pActiveTrackState->timeElapsedInt+=TimeAdd;
   }

  //check if we have end of sequence
  //on all tracks
  if(endOfSequence==TRUE)
  {
    onEndSequence();
    endOfSequence=FALSE;
    amTrace("End of Sequence"NL);
  }

  }
  else
  {
    // not playing
    // check sequence state if stopped reset position on all tracks
    // and reset tempo to default, but only once

    if(bStopped!=TRUE)
    {
      bStopped = TRUE;
      pActiveTrackState=0;
          
      // reset track
      pTrack = g_CurrentSequence->arTracks[0];

      pActiveTrackState=&(pTrack->currentState);
      pActiveTrackState->currentTempo=DEFAULT_MPQN;
      pActiveTrackState->currentBPM=DEFAULT_BPM;
      pActiveTrackState->timeElapsedInt=0L;
    }

    g_CurrentSequence->timeElapsedFrac=0L;
    g_CurrentSequence->timeStep=0L;

    // reset tempo to initial valueas taken during start(get them from main sequence?)
    g_CurrentSequence->timeStep = amCalculateTimeStep(pActiveTrackState->currentBPM,g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);

    //rewind to the first event
    while(pActiveTrackState->currEventPtr->pPrev!=0)
    {
      pActiveTrackState->currEventPtr = pActiveTrackState->currEventPtr->pPrev;
    }

    return;
  }

} //end UpdateStep()

// multitrack handler
void updateStepMulti(void)
{
    bStopped=FALSE;

    if(g_CurrentSequence==0) return;

    uint8 numOfTracks=g_CurrentSequence->ubNumTracks;

// get status from first track
    pTrack=g_CurrentSequence->arTracks[0];
    pActiveTrackState=&(pTrack->currentState);

    //check sequence state if paused do nothing
     if(pActiveTrackState->playState&TS_PS_PAUSED) 
     {
       amAllNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif
       return;
     }

     if(pActiveTrackState->playState&TS_PS_PLAYING){
        bStopped=FALSE;
     }
     else
     {


     }

     if(!(pActiveTrackState->playState&TS_PS_PLAYING)){
        //check sequence state if stopped reset position on all tracks
        //and reset tempo to default, but only once

        if(bStopped!=TRUE)
        {
             bStopped=TRUE;
             pActiveTrackState=0;
             pTrack=0;

             //reset track
             pTrack=g_CurrentSequence->arTracks[0];

             if(pTrack){
                   pActiveTrackState=&(pTrack->currentState);
                   pActiveTrackState->currentTempo=DEFAULT_MPQN;
                   pActiveTrackState->currentBPM=DEFAULT_BPM;
                   pActiveTrackState->timeElapsedInt=0L;
              }

             g_CurrentSequence->timeElapsedFrac=0L;
             g_CurrentSequence->timeStep=0L;

             //reset tempo to initial valueas taken during start(get them from main sequence?)
             g_CurrentSequence->timeStep=amCalculateTimeStep(pActiveTrackState->currentBPM,g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);

             //rewind to the first event
             while(pActiveTrackState->currEventPtr->pPrev!=0){
                 pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pPrev;
             }
        }
       return;
     }

     bStopped=FALSE; //we replaying, so we have to reset this flag

     // repeat for each track
     for(uint16 iTrackNb=0;iTrackNb<numOfTracks;++iTrackNb)
     {

      pTrack=g_CurrentSequence->arTracks[iTrackNb];
      pActiveTrackState=&(pTrack->currentState);

      if(pTrack)
      {
        pCurrentEvent=pActiveTrackState->currEventPtr;
        volatile sEventBlock_t *evntBlock= &pCurrentEvent->eventBlock;

        if(pCurrentEvent)
        {
          bEOTflag = (Bool)(evntBlock->type == T_META_EOT);
          timeElapsed = pActiveTrackState->timeElapsedInt;

          //reset
           bEventSent=FALSE;
                
           if(bEOTflag!=TRUE && pCurrentEvent->eventBlock.uiDeltaTime==timeElapsed )
           {
             endOfSequence=FALSE;
             //handle event
             // execute callback which copies data to midi buffer (_MIDIsendBuffer) (if IKBD_MIDI_SEND_DIRECT is defined)
             // execute callback which sends data directly to midi out (XBIOS) (otherwise)
             myFunc = pCurrentEvent->eventBlock.eventCb.func;
             (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);

             //go to next event
             pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pNext;
             pCurrentEvent=pActiveTrackState->currEventPtr;

            //check end of track
            if(pCurrentEvent!=0)
            {
              bEOTflag = (Bool)(pCurrentEvent->eventBlock.type == T_META_EOT);

              //check if next events are null and pack buffer until first next non zero delta
              while( (pCurrentEvent!=0) && ((bEOTflag==TRUE) && (pCurrentEvent->eventBlock.uiDeltaTime==0)) )
              {

                //handle event
                // execute callback which copies data to midi buffer (_MIDIsendBuffer) (if IKBD_MIDI_SEND_DIRECT is defined)
                // execute callback which sends data directly to midi out (XBIOS) (otherwise)
                myFunc = pCurrentEvent->eventBlock.eventCb.func;
                (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);

                //go to next event
                pActiveTrackState->currEventPtr = pActiveTrackState->currEventPtr->pNext;
                pCurrentEvent = pActiveTrackState->currEventPtr;

                if(pCurrentEvent)
                  bEOTflag = (Bool)(pCurrentEvent->eventBlock.type == T_META_EOT); 
              } //end while

              if(bEOTflag==TRUE)
              {
                endOfSequence=TRUE;
              }

              bEventSent=TRUE;
            }
          } //endif

      g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
      TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
      g_CurrentSequence->timeElapsedFrac &= 0xffff;

      if(TimeAdd>1)TimeAdd=1;

      //add time elapsed
        if(bEventSent==TRUE){
            pActiveTrackState->timeElapsedInt=0;
        }else{
            pActiveTrackState->timeElapsedInt=pActiveTrackState->timeElapsedInt+TimeAdd;
        }
       } // cur event null check
      } //end pTrack null check
     } // repeat for each track >> end

     //check if we have end of sequence
     //on all tracks
  if(endOfSequence == TRUE)
  {
    onEndSequence();
    endOfSequence = FALSE;
    amTrace("End of Sequence"NL);
  }

}

// replay control
Bool isAmSequencePlaying(void)
{
  AssertMsg(g_CurrentSequence!=0, "Current sequence is NULL");
  sTrack_t *pTrack=0;
  uint8 activeTrack=g_CurrentSequence->ubActiveTrack;
  pTrack=g_CurrentSequence->arTracks[activeTrack];

  if(pTrack)
  {
    if(pTrack->currentState.playState&TS_PS_PLAYING)
      return TRUE;
    else
      return FALSE;
  }
  
  return FALSE;
}


void stopAmSequence(void)
{
  sTrack_t *pTrack=0;
  AssertMsg(g_CurrentSequence!=0, "Current sequence is NULL");
  
  uint8 activeTrack=g_CurrentSequence->ubActiveTrack;
  pTrack=g_CurrentSequence->arTracks[activeTrack];

  if(pTrack)
  {
    const sTrackState_t *tState=&(pTrack->currentState);
    
      if( ((tState->playState&TS_PS_PLAYING)||(tState->playState&TS_PS_PAUSED)) )
      {
        pTrack->currentState.playState&=(~(TS_PS_PLAYING|TS_PS_PAUSED));
        amPrintf("Stop sequence"NL);
      }
  }
  

  //all notes off
  amAllNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
  Supexec(flushMidiSendBuffer);
#endif

}

void pauseAmSequence(void)
{
  AssertMsg(g_CurrentSequence!=0, "Current sequence is NULL");
  
  //amPrintf("Pause/Resume."NL);
  uint8 activeTrack=0;
	sTrack_t *pTrack=0;
  // TODO: handling individual tracks for MIDI 2 type
  // for one sequence( single / multichannel) we will check state of the first track only
  activeTrack = g_CurrentSequence->ubActiveTrack;
  pTrack = g_CurrentSequence->arTracks[activeTrack];

  if(pTrack)
  {
    const uint16 state = pTrack->currentState.playState;

    if((state&TS_PS_PLAYING)&&(!(state&TS_PS_PAUSED)))
    {
      pTrack->currentState.playState&=(uint16)(~TS_PS_PLAYING);
      pTrack->currentState.playState|=(uint16)TS_PS_PAUSED;

      amPrintf("Pause sequence"NL);
      return;
    }
    else if( !(state&TS_PS_PLAYING)&&(state&TS_PS_PAUSED) )
    {
      pTrack->currentState.playState&=(uint16)(~TS_PS_PAUSED); //unpause
      pTrack->currentState.playState|=(uint16)TS_PS_PLAYING;  //set playing state
      amPrintf("Resume sequence"NL);
    }
  }

}//pauseSeq

void playAmSequence(void)
{
   AssertMsg(g_CurrentSequence!=0, "Current sequence is NULL");
   //set state
   uint8 activeTrack = g_CurrentSequence->ubActiveTrack;
   sTrack_t *pTrack = g_CurrentSequence->arTracks[activeTrack];

   if(pTrack)
   {
     const uint16 state=pTrack->currentState.playState;

     if(!(state&TS_PS_PLAYING)) 
     {
       pTrack->currentState.playState&=(~(TS_PS_PAUSED));
       pTrack->currentState.playState|=TS_PS_PLAYING;

       amPrintf("Play sequence\t");

       if(pTrack->currentState.playState&TM_PLAY_ONCE)
       {
          amPrintf("[ ONCE ]"NL);
       }
       else
       {
         amPrintf("[ LOOP ]"NL);
       }
      }
  }
}

void muteAmTrack(const uint16 trackNb, const Bool bMute)
{
  AssertMsg(g_CurrentSequence!=0, "Current sequence is NULL");
  AssertMsg(trackNb<AMIDI_MAX_TRACKS, "Invalid track id");

 if(bMute==TRUE)
 {
    g_CurrentSequence->arTracks[trackNb]->currentState.playState|=TM_MUTE;
    amPrintf("Mute track %d"NL,trackNb);
  }
  else
  {
    g_CurrentSequence->arTracks[trackNb]->currentState.playState&=(~TM_MUTE);
    amPrintf("UnMute track %d"NL,trackNb);
  }
  
}

void toggleAmReplayMode(void)
{
  AssertMsg(g_CurrentSequence!=0, "Current sequence is NULL");

  const uint8 activeTrack = g_CurrentSequence->ubActiveTrack;
  sTrack_t *pTrack = g_CurrentSequence->arTracks[activeTrack];
  const uint16 state = pTrack->currentState.playState;

  if(state&TM_PLAY_ONCE)
  {
    pTrack->currentState.playState&=(~TM_PLAY_ONCE);
    amPrintf("Set replay mode: [ LOOP ]"NL);
  }
  else
  {
    pTrack->currentState.playState|=TM_PLAY_ONCE;
    amPrintf("Set replay mode: [ ONCE ]"NL);
  }
  
}

void destroyAmSequence (sSequence_t **pPtr)
{
  #ifdef DEBUG_BUILD
    amTrace((const uint8 *)"destroyAmSequence() destroy sequence at %p initiated... 1..2..3... "NL,*pPtr);
  #endif

  sSequence_t *seq = (*pPtr);
  
  AssertMsg(seq!=0, "Current sequence is NULL");

  // go to the end of sequence
  if(seq->pSequenceName!=0){
      gUserMemFree((seq->pSequenceName),0);
  }

  // destroy all tracks
   for (uint16 i=0;i<AMIDI_MAX_TRACKS;++i)
   {
     if(seq->arTracks[i]!=0)
     {
     if(seq->arTracks[i]->pTrackName!=0) gUserMemFree((seq->arTracks[i]->pTrackName),0);

#ifdef EVENT_LINEAR_BUFFER
      destroyList(seq,&(seq->arTracks[i]->pTrkEventList));
#else
      destroyList(&(seq->arTracks[i]->pTrkEventList));
#endif

      gUserMemFree((seq->arTracks[i]),0);
     }
   }

  //destroy sequence and nullify it
  gUserMemFree(seq,0);
  
  #ifdef DEBUG_BUILD
    amTrace((const uint8 *)"destroyAmSequence() done. "NL);
  #endif
  seq = 0;
}

void printAmSequenceState(void)
{

if(g_CurrentSequence)
{
  amPrintf("Td/PPQN: %u"NL,g_CurrentSequence->timeDivision);
  amPrintf("Time step: %u"NL,g_CurrentSequence->timeStep);
  amPrintf("Time elapsedFrac: %u"NL,g_CurrentSequence->timeElapsedFrac);

  sTrack_t *pTrack=0;

    switch(g_CurrentSequence->seqType)
    {

    case ST_SINGLE:
    {
      pTrack = g_CurrentSequence->arTracks[0];

      if(pTrack)
      {
		    sTrackState_t *pTrackState=0;
        amPrintf("Track state:"NL);
		 
        pTrackState=&(pTrack->currentState);
        amPrintf("\tTime elapsed: %u"NL,pTrackState->timeElapsedInt);
        amPrintf("\tCur BPM: %u"NL,pTrackState->currentBPM);
        amPrintf("\tCur Tempo: %u"NL,pTrackState->currentTempo);
        amPrintf("\tCur Play state: %s"NL,getPlayStateStr(pTrackState->playState));
        amPrintf("\tMute: %s"NL,(pTrackState->playState & TM_MUTE)?"TRUE":"FALSE");
      }
    } break;
    case ST_MULTI:
    {
		  pTrack=g_CurrentSequence->arTracks[g_CurrentSequence->ubActiveTrack];
      sTrackState_t *pTrackState=&(pTrack->currentState);

      amPrintf("Nb of tracks: %d"NL,g_CurrentSequence->ubNumTracks);
      amPrintf("Active track: %d"NL,g_CurrentSequence->ubActiveTrack);
      amPrintf("Cur Play state: %s"NL,getPlayStateStr(pTrackState->playState));
      amPrintf("Cur Tempo: %u"NL,pTrackState->currentTempo);
      amPrintf("Cur BPM: %u"NL,pTrackState->currentBPM);

      for (uint16 i=0;i<g_CurrentSequence->ubNumTracks;++i)
      {
        pTrack=g_CurrentSequence->arTracks[i];

         if(pTrack)
         {
            amPrintf("Track[%d]\t",i);
            pTrackState=&(pTrack->currentState);
            amPrintf("\tTime elapsed: %u\t",pTrackState->timeElapsedInt);
            amPrintf("\tMute: %s"NL,pTrackState->playState&TM_MUTE?"TRUE":"FALSE");
          }
      }
    }break;
    case ST_MULTI_SUB:
    {
      amPrintf("Nb of tracks: %d"NL,g_CurrentSequence->ubNumTracks);
      amPrintf("Active track: %d"NL,g_CurrentSequence->ubActiveTrack);

      for (uint16 i=0;i<g_CurrentSequence->ubNumTracks;++i)
      {
        pTrack=g_CurrentSequence->arTracks[i];
				sTrackState_t *pTrackState=0;
        amPrintf("Track[%d]"NL,i);
        pTrackState=&(pTrack->currentState);
        amPrintf("Time elapsed: %u"NL,pTrackState->timeElapsedInt);
        amPrintf("Cur BPM: %u"NL,pTrackState->currentBPM);
        amPrintf("Cur Tempo: %u"NL,pTrackState->currentTempo);
        amPrintf("Cur play state: %s"NL,getPlayStateStr(pTrackState->playState));
        amPrintf("\tMute: %s"NL,pTrackState->playState&TM_MUTE?"TRUE":"FALSE");
       }
      } break;
  }; // switch()
 
#ifdef DEBUG_BUILD
	printMidiSendBufferState();
#endif
}

}

const uint8 *getPlayStateStr(const uint16 state)
{
    if( !(state&TS_PS_PLAYING) && (state&TS_PS_PAUSED) ){
       return "Paused";
    }else if(state&TS_PS_PLAYING && (!(state&TS_PS_PAUSED))){
       return "Playing";
    }else if(!(state&TS_PS_PLAYING)){
       return "Stopped...";
    }
 AssertMsg(0,"Undefined state");	
 return "";
}




