
/**  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation\, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/


#include "INCLUDE/MIDISEQ.H"
#include "INCLUDE/AMIDILIB.H"
#include "INCLUDE/MIDI_CMD.H"	/* for sending midi commands */

#include <stdio.h> /* just for dummy printf in event handling messages */
				   /* remove this include when all the handlers will be implemented */
				   
/* string table with all event names */
/*  */

extern "C"{
  static U8 messBuf[256]; /* for error buffer  */
}


static const char *g_arEventNames[T_EVT_COUNT]=
{
	"Note On",
	"Note Off", 
	"Note Aftertouch", 
	"Controller",
	"Program Change", 
	"Channel Aftertouch", 
	"Pitch bend",
	"Set tempo(Meta)"
};

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
   {sizeof(sTempo_EventBlock_t),fSetTempo}
};

sEventInfoBlock_t getEventFuncInfo(U8 eventType)
{
	sEventInfoBlock_t infoBlock;
	infoBlock.size=g_arSeqCmdTable[eventType].size;
	infoBlock.func=g_arSeqCmdTable[eventType].func;
	return infoBlock;
}


/*returns pointer to NULL terminated string with event name */
/* id is enumerated value from eEventType */
const U8 *getEventName(U32 id){
	return ((const U8 *)g_arEventNames[id]);
}


void  fNoteOn(void *pEvent){
	sNoteOn_EventBlock_t *pPtr=(sNoteOn_EventBlock_t *)pEvent;
	sprintf((char *)messBuf, "Sending Note On data to sequencer ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
	am_log(messBuf);
	note_on(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);	
}

void  fNoteOff(void *pEvent){
	sNoteOff_EventBlock_t *pPtr=(sNoteOff_EventBlock_t *)pEvent;
 	sprintf((char *)messBuf, "Sending Note Off data to sequencer ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
	am_log(messBuf);
	note_off(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

void  fNoteAft(void *pEvent){
	sNoteAft_EventBlock_t *pPtr=(sNoteAft_EventBlock_t *)pEvent;	
	sprintf((char *)messBuf, "Sending Note Aftertouch data to sequencer ch:%d note:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
	am_log(messBuf);
	polyphonic_key_press(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
}

void  fProgramChange (void *pEvent){
	sPrgChng_EventBlock_t *pPtr=(sPrgChng_EventBlock_t *)pEvent;
	sprintf((char *)messBuf, "Sending Program change data to sequencer ch:%d pn:%d...\n",pPtr->ubChannelNb,pPtr->eventData.programNb);
	am_log(messBuf);
	program_change(pPtr->ubChannelNb,pPtr->eventData.programNb);
}

void  fController(void *pEvent){
	sController_EventBlock_t *pPtr=(sController_EventBlock_t *)pEvent;
	sprintf((char *)messBuf, "Sending Controller data to sequencer ch:%d controller:%d value:%d...\n",pPtr->ubChannelNb,pPtr->eventData.controllerNb,pPtr->eventData.value);
	am_log(messBuf);
	control_change(pPtr->eventData.controllerNb, pPtr->ubChannelNb, pPtr->eventData.value,0x00);
}

void  fChannelAft(void *pEvent){
	sChannelAft_EventBlock_t *pPtr=(sChannelAft_EventBlock_t *)pEvent;
	sprintf((char *)messBuf, "Sending Channel Aftertouch data to sequencer ch:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.pressure);
	am_log(messBuf);
	channel_pressure (pPtr->ubChannelNb,pPtr->eventData.pressure);

}

void  fPitchBend(void *pEvent){
	sPitchBend_EventBlock_t *pPtr=(sPitchBend_EventBlock_t *)pEvent;
	sprintf((char *)messBuf, "Sending Pitch bend data to sequencer ch:%d LSB:%d MSB:%d...\n",pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
	am_log(messBuf);
	pitch_bend_2 (pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
}

void  fSetTempo(void *pEvent){
sTempo_EventBlock_t *pPtr=(sTempo_EventBlock_t *)pEvent;	
	sprintf((char *)messBuf, "Setting new replay tempo...\n");
	am_log(messBuf);
}


