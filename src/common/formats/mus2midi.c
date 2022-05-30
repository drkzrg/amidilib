/*
===========================================================================
Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").  

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================

/|\ m68k / atari version / cleanup / customisation:
Copyright (C) 2007-2021 Pawel Goralski

===========================================================================
*/

#include <assert.h>
#include <stdlib.h>
#include <amlog.h>
#include "core/amprintf.h"

#include <dmus.h>
#include <midi.h> 
#include "memory/memory.h"
#include "memory/endian.h"

#ifdef ENABLE_GEMDOS_IO
#include <mint/ostruct.h>
#include <mint/osbind.h>
#include "gemdosio.h"
#endif

#define MUSEVENT_KEYOFF	0
#define MUSEVENT_KEYON	1
#define MUSEVENT_PITCHWHEEL	2
#define MUSEVENT_CHANNELMODE	3
#define MUSEVENT_CONTROLLERCHANGE	4
#define MUSEVENT_END	6


// writes a byte, and returns the buffer
uint8* WriteByte(void* buf, int8 b)
{
    uint8* buffer = (uint8*)buf;
    *buffer = b;
    ++buffer;
    return buffer;
}

uint8* WriteShort(void* b, uint16 s)
{
    uint16* buffer = (uint16*)b;
    *buffer = s;
    ++buffer;
    return (uint8 *)buffer;
}

uint8* WriteInt(void* b, uint32 i)
{
 uint32* buffer = (uint32*)b;
 *buffer = i;
 ++buffer;
 return (uint8 *)buffer;
}

int32 UpdateBytesWritten(int32* bytes_written, int32 to_add, int32 max)
{
    *bytes_written += to_add;

    if(max!=-1){
        if (max && *bytes_written > max){
            return 0;
        }
    } 
    
  return 1;
}

// we are making format 0, nb of tracks==1, div=0xe250(?)

// Format - 0 (1 track only)
void Midi_CreateHeader(sMThd* header){
	WriteInt(&header->id,ID_MTHD);
	WriteInt(&header->headLenght, 6);
    WriteShort(&header->format, 0);
    WriteShort(&header->nTracks, 1);
    WriteShort(&header->division, 0x0059);
}

unsigned char* Midi_WriteTempo(unsigned char* buffer){
    buffer = WriteByte(buffer, 0x00);           // delta time
    buffer = WriteByte(buffer, EV_META);        // meta event
    buffer = WriteByte(buffer, MT_SET_TEMPO);   // set tempo
    buffer = WriteByte(buffer, 0x3);            // set tempo

    //0x001aa309
    //->09a31a
    buffer = WriteByte(buffer, 0x09);
    buffer = WriteByte(buffer, 0xa3);
    buffer = WriteByte(buffer, 0x1a);

    amTrace("Midi_WriteTempo() Mus2Midi: 0x%lx"NL,0x00FFFFFF&*((uint32 *)(buffer-4)));

	return buffer;
}


static const uint8 MidiMap[] = {
    0,		//      prog change
    0,		//      bank sel
    1,      //2		// mod pot
	0x07,	//3		// volume
	0x0A,	//4		// pan pot
	0x0B,	//5		// expression pot
	0x5B,	//6		// reverb depth
	0x5D,	//7		// chorus depth
	0x40,	//8		// sustain pedal
	0x43,	//9		// soft pedal
    0x78,	//10	// all sounds off
    0x7B,	//11	// all notes off
    0x7E,	//12	// mono(use numchannels + 1)
    0x7F,	//13	// poly
    0x79,	//14	// reset all controllers
};

retVal Mus2Midi(uint8* bytes, uint8* out, const int8 *pOutMidName,uint32* len)
{
// mus header and instruments
MUSheader_t header;

// current position in read buffer
uint8* cur = bytes,*end;

// Midi header(format 0)
sMThd midiHeader;
// Midi track header, only 1 needed(format 0)
sChunkHeader midiTrackHeader;
// Stores the position of the midi track header(to change the size)
uint8* midiTrackHeaderOut=0;

//zero mem
amMemSet(&midiHeader,0,sizeof(sMThd));
amMemSet(&midiTrackHeader,0,sizeof(sChunkHeader));
amMemSet(&header,0,sizeof(MUSheader_t));

// Delta time for midi event
int32 delta_time = 0;
uint16 temp=0;
int32 channel_volume[MIDI_MAXCHANNELS] = {0};
int32 bytes_written = 0;
int32 channelMap[MIDI_MAXCHANNELS], currentChannel = 0;

// read the mus header
amMemCpy(&header, cur, sizeof(MUSheader_t));
cur += sizeof(MUSheader_t);

header.scoreLen = ReadLE16( header.scoreLen );
header.scoreStart = ReadLE16( header.scoreStart );
header.channels = ReadLE16( header.channels );
header.sec_channels = ReadLE16( header.sec_channels );
header.instrCnt = ReadLE16( header.instrCnt );
header.dummy = ReadLE16( header.dummy );

// only 15 supported
if (header.channels > MIDI_MAXCHANNELS - 1) {

#ifndef SUPRESS_CON_OUTPUT
    amPrintf("[Error] Too many channels, only 15 is supported."NL);
#endif

    amTrace("[Error] Too many channels, only 15 is supported."NL);
    return AM_ERR;
}

 amTrace("MUS info:"NL);
 amTrace("Score length 0x%x"NL,header.scoreLen);
 amTrace("Score start 0x%x"NL,header.scoreStart);
 amTrace("Nb of channels 0x%x"NL,header.channels);
 amTrace("sec_channels 0x%x"NL,header.sec_channels);
 amTrace("Nb of instruments: 0x%x"NL,header.instrCnt);

  // Map channel 15 to 9(percussions)
  for (temp = 0; temp < MIDI_MAXCHANNELS; ++temp) 
  {
		channelMap[temp] = -1;
		channel_volume[temp] = 0x40;
   }

    channelMap[15] = 9;

	// Get current position, and end of position
	cur = bytes + header.scoreStart;
	end = cur + header.scoreLen;

    // Write out midi header format 0
    Midi_CreateHeader(&midiHeader);
    UpdateBytesWritten(&bytes_written, sizeof(sMThd), *len);
    amMemCpy(out, &midiHeader, sizeof(sMThd));
    out += sizeof(sMThd);

	// Store this position, for later filling in the midiTrackHeader
    UpdateBytesWritten(&bytes_written, sizeof(sChunkHeader), *len);
	midiTrackHeaderOut = out;
    out += sizeof(sChunkHeader);

    // microseconds per quarter note (yikes)
    UpdateBytesWritten(&bytes_written, 7, *len);
    out = Midi_WriteTempo(out);

	// Percussions channel starts out at full volume
    UpdateBytesWritten(&bytes_written, 4, *len);
    out = WriteByte(out, 0x00); //delta
    out = WriteByte(out, 0xB9); //channel 9
    out = WriteByte(out, 0x07);
	out = WriteByte(out, 127);

	// Main Loop
    uint8 temp_buffer[32];	// temp buffer for current iterator
    uint8 *out_local=0;
    
	while (cur < end) {
    uint8 event;
	uint8 channel=0;
	uint8 status=0, bit1=0, bit2=0, bitc = 2;

    status=0, bit1=0, bit2=0, bitc = 2;
    out_local = temp_buffer;

		// Read in current bit
		event		= *cur++;
		channel		= (event & 15);		// current channel

		// Write variable length delta time
		out_local += WriteVarLen(delta_time, out_local);

		if (channelMap[channel] < 0) {
			// Set all channels to 127 volume
			out_local = WriteByte(out_local, 0xB0 + currentChannel);
			out_local = WriteByte(out_local, 0x07);
			out_local = WriteByte(out_local, 127);
			out_local = WriteByte(out_local, 0x00);

			channelMap[channel] = currentChannel++;
			if (currentChannel == 9)
				++currentChannel;
		}

		status = channelMap[channel];

        // Handle events
		switch ((event & 122) >> 4)
		{
		default:
            AssertMsg(0,"Unsupported event");
		break;
		case MUSEVENT_KEYOFF:
			status |=  0x80;
			bit1 = *cur++;
			bit2 = 0x40;
			break;
		case MUSEVENT_KEYON:
			status |= 0x90;
			bit1 = *cur & 127;
			if (*cur++ & 128)	// volume bit?
				channel_volume[channelMap[channel]] = *cur++;
			bit2 = channel_volume[channelMap[channel]];
			break;
		case MUSEVENT_PITCHWHEEL:
			status |= 0xE0;

			// pitch wheel range should be $0-$3fff, TODO: add validation / assert
            bit1 = (*cur & 1) >> 6;
            bit2 = (*cur++ >> 1) & 127;
		break;
		case MUSEVENT_CHANNELMODE:
			status |= 0xB0;
            Assert(*cur < sizeof(MidiMap) / sizeof(MidiMap[0]));
			bit1 = MidiMap[*cur++];
			bit2 = (*cur++ == 12) ? header.channels + 1 : 0x00;
			break;
		case MUSEVENT_CONTROLLERCHANGE:
			if (*cur == 0) {
				cur++;
				status |= 0xC0;
				bit1 = *cur++;
				bitc = 1;
			} else {
				status |= 0xB0;
              	Assert((int32)(*cur < sizeof(MidiMap) / sizeof(MidiMap[0])));
				bit1 = MidiMap[*cur++];
				bit2 = *cur++;
			}
			break;
		case 5:	// Unknown
            AssertMsg(0,"Unknown event");
		break;
		case MUSEVENT_END:	// End
			status = 0xff;
			bit1 = 0x2f;
			bit2 = 0x00;
            Assert(cur == end);
			break;
		case 7:	// Unknown
            AssertMsg(0,"Unknown event");
			break;
		}

		// Write it out
		out_local = WriteByte(out_local, status);
		out_local = WriteByte(out_local, bit1);

        if (bitc == 2) out_local = WriteByte(out_local, bit2);

		// Write out temp stuff
		if (out_local != temp_buffer){
            UpdateBytesWritten(&bytes_written, out_local - temp_buffer, *len);
            amMemCpy(out, temp_buffer, out_local - temp_buffer);
			out += out_local - temp_buffer;
		}

		if (event & 128) {
			delta_time = 0;
			do {
				delta_time = delta_time * 128 + (*cur & 127);
			} while ((*cur++ & 128));
		} else {
			delta_time = 0;
		}
	}

	// Write out track header
	WriteInt(&midiTrackHeader.id, ID_MTRK);
    WriteInt(&midiTrackHeader.headLenght, out - midiTrackHeaderOut - sizeof(sChunkHeader));
	
    amMemCpy(midiTrackHeaderOut, &midiTrackHeader, sizeof(sChunkHeader));

	// Store length written
	*len = bytes_written;

#ifndef SUPRESS_CON_OUTPUT
    amPrintf("bytes written %u"NL,(uint32)(*len));
#endif

    amTrace("bytes written %u"NL,(uint32)(*len));

     if(pOutMidName){

#ifndef SUPRESS_CON_OUTPUT
       amPrintf("Writing MIDI output to file: %s"NL, pOutMidName);
#endif

        amTrace("Writing MIDI output to file: %s"NL, pOutMidName);

#ifdef ENABLE_GEMDOS_IO
        int16 fileHandle = Fcreate(pOutMidName, 0);

        if(fileHandle>0){
            amTrace("[GEMDOS] Create file, gemdos handle: %d"NL,fileHandle);

            int32 bytesWritten = Fwrite(fileHandle, bytes_written, midiTrackHeaderOut - sizeof(sMThd));
            amTrace("Saved to file: [%d] bytes to gemdos handle %d. "NL, bytesWritten,fileHandle);

            amTrace("Closing gemdos handle %d "NL, fileHandle);
            int16 err=Fclose(fileHandle);

            if(err!=GDOS_OK){
              amTrace("[GEMDOS] Error closing file handle : [%d] "NL, fileHandle, getGemdosError(err));
            }

#ifndef SUPRESS_CON_OUTPUT
        amPrintf("Written %d bytes"NL,bytes_written);
#endif

        amTrace("Written %d bytes"NL,bytes_written);

        }else{
            #ifndef SUPRESS_CON_OUTPUT
                amPrintf("[GEMDOS] Error: %s Couldn't create midi output file: %s"NL,getGemdosError(fileHandle), pOutMidName);
            #endif

            amTrace("[GEMDOS] Error: %s Couldn't create midi output file: %s"NL,getGemdosError(fileHandle), pOutMidName);
        }

#else

        FILE* file = fopen(pOutMidName, "wb");
        fwrite(midiTrackHeaderOut - sizeof(sMThd), bytes_written, 1, file);
        fclose(file);

#ifndef SUPRESS_CON_OUTPUT
       amPrintf("Written %d bytes"NL,bytes_written);
#endif
       amTrace("Written %d bytes"NL,bytes_written);
#endif

      }// end of midi output file write

#ifndef SUPRESS_CON_OUTPUT
    amPrintf( "Done. [OK]"NL);
#endif

 amTrace("Done. OK"NL);
 return AM_OK;
}
