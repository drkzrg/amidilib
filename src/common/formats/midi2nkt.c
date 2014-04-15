
#include "nkt.h"

#include "events.h"
#include "timing/miditim.h"
#include "memory/linalloc.h"
#include "midi.h"

void Nkt_CreateHeader(sNktHd* header, const sMThd *pMidiHeader, const BOOL bCompress){
    WriteInt(&header->id,ID_NKT);
    WriteInt(&header->NbOfBlocks, 0);
    WriteShort(&header->division, pMidiHeader->division);
    WriteShort(&header->bPacked, bCompress);
    WriteShort(&header->version, 1);
}


void *midiTrackDataToFile(void *startPtr, void *endPtr, FILE **file, U32 *blocks_written, U32 *bytes_written, BOOL *error){

}

S32 Midi2Nkt(const void *pMidiData, const U8 *pOutFileName, const BOOL bCompress){
U32 bytes_written = 0;
U32 blocks_written = 0;
BOOL error=FALSE;
FILE* file=0;
sNktHd nktHead;

if(pOutFileName){
   // create file header
   amTrace("Writing NKT file to: %s\n",pOutFileName);
   Nkt_CreateHeader(&nktHead, (const sMThd *)pMidiData, bCompress);

   file = fopen(pOutFileName, "wb");
   bytes_written+=fwrite(&nktHead, sizeof(sNktHd), 1, file);
}

/* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
void *startPtr=(void *)((U32)pMidiData+sizeof(sMThd));
void *endPtr=0;

while (startPtr!=0 && error!=FALSE){
  // process event and store them into file
  startPtr = midiTrackDataToFile(startPtr, endPtr, &file, &blocks_written,&bytes_written, &error);

}

if(file){
   fseek(file, 0, SEEK_SET);
   // update header
   fwrite(&nktHead, sizeof(sNktHd), 1, file);

   fclose(file); file=0;
   amTrace("Stored %d event blocks, %lu kb(%lu bytes) of data.\n",nktHead.NbOfBlocks,nktHead.NbOfBytesData/1024,nktHead.NbOfBytesData);
 }

}
