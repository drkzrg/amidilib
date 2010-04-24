/*  Copyright 2008, 2009 Pawel Goralski
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "INCLUDE/AMIDILIB.H"
#include "INCLUDE/LIST/LIST.H"

/* test midi file to load */
/*#define MIDI_FILE "..\\TUNES\\ULTIMA30.MID"*/
#define MIDI_FILE "TUNES/ULTIMA01.MID"
#define XMIDI_FILE "TUNES/UWR10.XMI"

/**
 * main program entry
 */

int main(void)
{
    S32 lRet=0L;
    U32 currDelta=0,lastDelta=0;
    void *pMidi=NULL;
    sSequence_t midiTune;
    U16 iRet=0;
    U32 ulFileLenght=0L;
    S16 iError=0;
    U32 mem=0;
    const sEventList *pMyEvent=NULL;	

    /*VLQ test */
    U32 val[]={0x00, 0x7F,0x8100,0xC000,0xFF7F,0x818000, 0xFFFF7F,0x81808000,0xC0808000,0xFFFFFF7F };
    U32 result=0,iCounter;
    U8 *pValPtr=NULL;
    U8 valsize;
    
    for (iCounter=0;iCounter<10;iCounter++)   {
        pValPtr=(U8 *)(&val[iCounter]);
        while((*pValPtr)==0x00)
        {pValPtr++;}
        valsize=0;result=0;
        result= readVLQ(pValPtr,&valsize);
     	printf("VLQ value:%lx, decoded: %lx, size: %d\n",val[iCounter], result, valsize );
    }
    
    /* mem tst */
    mem=getFreeMem(ST_RAM);
    mem=getFreeMem(TT_RAM);
    mem=getFreeMem(PREFER_ST);
    mem=getFreeMem(PREFER_TT);
  

/* init library */
    iError=am_init();
    
	/* get connected device info */
	getConnectedDeviceInfo();

    pMidi=loadFile((U8 *)MIDI_FILE, PREFER_TT, &ulFileLenght);

    if(pMidi!=NULL)
    {

				 	printf("Midi loaded in the memory succesfully...\n");
                    /* process MIDI*/
					/* check midi header */
		   iRet=am_getHeaderInfo(pMidi);

		   if(iRet>0){			
                    // no error
		    switch(iRet)
                    {

                        

                        case 0:
						case 1:
						case 2:
						case 3:
                        /* handle file */
                        am_handleMIDIfile(pMidi, iRet, ulFileLenght,&midiTune);
                        break;

			default:
                            /* unknown error, do nothing */
                            printf("Unknown error ...\n");
                            iError=1;
                          break;

                    }
		   }
		   else{
		     if(iRet== -1){
                      /* not MIDI file, do nothing */
                      printf("It's not valid MIDI file...\n");
                    }
                     else if(iRet==-2){
		     /* unsupported MIDI type format, do nothing*/
                      printf("Unsupported MIDI file format...\n");
                     }
		     iError=1;
		   }
                    if(iError!=1)
		    {
                        /* play preloaded tune */
                        ;
                    }

				}
				else
					{
						printf("Error: Couldn't read file...\n");
						return(-1);
					}

       
		pMyEvent= &((midiTune.arTracks[0])->trkEventList); 
		 /* play our sequence - send all events  */		      
		/*printEventList( &pMyEvent );*/
		currDelta=0;
		lastDelta=0;
		printf("Sending all events with delta: %ld\n", currDelta);
		/*getchar();*/
		
		/* TODO: callback on timer */
		
		while((currDelta=sendMidiEvents(lastDelta, &pMyEvent)))
		{
			
			printf("Sending all events with delta: %ld\n", currDelta);
			lastDelta=currDelta;
		}
		
		/* turn off all notes on all channels */
		all_notes_off(0);        
		all_notes_off(1); 
	        all_notes_off(2); 
		all_notes_off(3); 
		all_notes_off(4); 
	        all_notes_off(5); 
		all_notes_off(6); 
		all_notes_off(7); 
		all_notes_off(8); 
		all_notes_off(9); 
		all_notes_off(10); 
		all_notes_off(11); 
		all_notes_off(12); 
		all_notes_off(13); 
		all_notes_off(14); 
		all_notes_off(15); 
        
        /* free up buffer */
        if(pMidi!=NULL)	Mfree(pMidi);
        /* clean up, free internal library buffers etc..*/
        am_deinit();
    
    return (0);
}


