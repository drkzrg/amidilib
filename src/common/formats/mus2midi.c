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

m68k/ atari/ cleanup/ customisation: Pawel Goralski

*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <amlog.h>

#include <dmus.h>
#include <midi.h> 
#include <memory/memory.h>
#include <amidilib.h>

#define MUSEVENT_KEYOFF	0
#define MUSEVENT_KEYON	1
#define MUSEVENT_PITCHWHEEL	2
#define MUSEVENT_CHANNELMODE	3
#define MUSEVENT_CONTROLLERCHANGE	4
#define MUSEVENT_END	6
#define MIDI_MAXCHANNELS	16
#define MIDIHEADERSIZE 14

// we are making format 0, nb of tracks==1, div=0xe250(?)

// reads a variable length integer
// TODO: remove it and replace with U32 readVLQ(U8 *pChar,U8 *ubSize)
static U32 ReadVarLen(S8* buffer){
U32 value;
U8 c;

if ((value = *buffer++) & 0x80) {
  value &= 0x7f;
  do  {
    value = (value << 7) + ((c = *buffer++) & 0x7f);
  } while (c & 0x80);
 }
 return value;
}

// Writes a variable length integer to a buffer, and returns bytes written
static S32 WriteVarLen( S32 value, U8* out ){
    S32 buffer, count = 0;

	buffer = value & 0x7f;

    while ((value >>= 7) > 0) {
		buffer <<= 8;
		buffer += 0x80;
		buffer += (value & 0x7f);
	}

	while (1) {
		++count;
        *out = (U8)buffer;
		++out;
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
 }
 return count;
}

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

    amTrace("Midi_WriteTempo() Mus2Midi: 0x%lx\n",0x00FFFFFF&*((U32 *)(buffer-4)));

	return buffer;
}


U8 MidiMap[] = {
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

// The MUS data is stored in little-endian, m68k is big endian
U16 LittleToNative(const U16 value){
U16 val=value;
U16 result;
  
  result=value<<8;
  val=val>>8;
  result=result|val;
  return result;
}

S32 Mus2Midi(U8* bytes, U8* out, const S8 *pOutMidName,U32* len){
// mus header and instruments
MUSheader_t header;

// current position in read buffer
unsigned char* cur = bytes,*end;

// Midi header(format 0)
sMThd midiHeader;
// Midi track header, only 1 needed(format 0)
sChunkHeader midiTrackHeader;
// Stores the position of the midi track header(to change the size)
U8* midiTrackHeaderOut=0;

//zero mem
amMemSet(&midiHeader,0,sizeof(sMThd));
amMemSet(&midiTrackHeader,0,sizeof(sChunkHeader));
amMemSet(&header,0,sizeof(MUSheader_t));

// Delta time for midi event
S32 delta_time = 0;
S32 temp=0;
S32 channel_volume[MIDI_MAXCHANNELS] = {0};
S32 bytes_written = 0;
S32 channelMap[MIDI_MAXCHANNELS], currentChannel = 0;
U8 last_status = 0;

// read the mus header
amMemCpy(&header, cur, sizeof(MUSheader_t));
cur += sizeof(MUSheader_t);

header.scoreLen = LittleToNative( header.scoreLen );
header.scoreStart = LittleToNative( header.scoreStart );
header.channels = LittleToNative( header.channels );
header.sec_channels = LittleToNative( header.sec_channels );
header.instrCnt = LittleToNative( header.instrCnt );
header.dummy = LittleToNative( header.dummy );

// only 15 supported
if (header.channels > MIDI_MAXCHANNELS - 1) return 0;

 amTrace("ScoreLen 0x%x\n",header.scoreLen);
 amTrace("ScoreStart 0x%x\n",header.scoreStart);
 amTrace("channels 0x%x\n",header.channels);
 amTrace("sec_channels 0x%x\n",header.sec_channels);
 amTrace("instrCnt 0x%x\n",header.instrCnt);

  // Map channel 15 to 9(percussions)
  for (temp = 0; temp < MIDI_MAXCHANNELS; ++temp) {
		channelMap[temp] = -1;
		channel_volume[temp] = 0x40;
   }

    channelMap[15] = 9;

	// Get current position, and end of position
	cur = bytes + header.scoreStart;
	end = cur + header.scoreLen;

    // Write out midi header format 0
    Midi_CreateHeader(&midiHeader);
    UpdateBytesWritten(&bytes_written, MIDIHEADERSIZE, *len);
    amMemCpy(out, &midiHeader, MIDIHEADERSIZE);	// cannot use sizeof(packs it to 16 bytes)
	out += MIDIHEADERSIZE;

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
    U8 channel;
    U8 event;
    U8 temp_buffer[32];	// temp buffer for current iterator
    U8 *out_local=0;
    U8 status=0, bit1=0, bit2=0, bitc = 2;

	while (cur < end) {
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
		switch ((event & 122) >> 4){
		default:
			assert(0);
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
			bit1 = (*cur & 1) >> 6;
			bit2 = (*cur++ >> 1) & 127;
			break;
		case MUSEVENT_CHANNELMODE:
			status |= 0xB0;
			assert(*cur < sizeof(MidiMap) / sizeof(MidiMap[0]));
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
				assert(*cur < sizeof(MidiMap) / sizeof(MidiMap[0]));
				bit1 = MidiMap[*cur++];
				bit2 = *cur++;
			}
			break;
		case 5:	// Unknown
			assert(0);
			break;
		case MUSEVENT_END:	// End
			status = 0xff;
			bit1 = 0x2f;
			bit2 = 0x00;
			assert(cur == end);
			break;
		case 7:	// Unknown
			assert(0);
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

    amTrace("bytes written %d\n",*len);

     if(pOutMidName){
        amTrace("Writing MIDI output to file: %s\n",pOutMidName);
        FILE* file = fopen(pOutMidName, "wb");
        fwrite(midiTrackHeaderOut - sizeof(sMThd), bytes_written, 1, file);
        fclose(file);
        amTrace("Written %d bytes\n",bytes_written);
      }

 amTrace("Done. OK\n");
 return 1;
}
