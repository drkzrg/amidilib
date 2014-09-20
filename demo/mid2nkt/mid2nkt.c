
#include <stdio.h>
#include <string.h>

#include "dmus.h"           // MUS->MID conversion
#include "fmio.h"           // disc i/o
#include "amlog.h"          // logging
#include "midi.h"           // midi
#include "midi2nkt.h"

#define MIDI_OUT_TEMP (100*1024) // temporary buffer for MUS->MID conversion

void printInfoScreen();

int main(int argc, char *argv[]){

BOOL bEnableCompression=FALSE;
U8 *filePath=0;

  initDebug("mid2nkt.log");

  printInfoScreen();

  // check parameters for compression
  if(argc==2 && argv[1]!='\0'){
      fprintf(stderr,"Trying to load %s\n",argv[1]);
      filePath=argv[1];

  }else if(argc==3&&argv[1]!='\0'&&argv[2]!='\0'){

      if((strcmp("-c",argv[1])==0)||(strcmp("--compress",argv[1])==0)){
       bEnableCompression=TRUE;
      }

      fprintf(stderr,"Trying to load %s\n",argv[2]);
      filePath=argv[2];

  }else{
      fprintf(stderr,"No specified mid / mus filename or bad parameters! Exiting ...\n");
      deinitDebug();
      getchar();
      return 0;
  }

  // load midi file into memory
  U32 iError=0;
  U32 ulFileLenght=0L;
  void *pMidi=0;

  pMidi=loadFile(filePath, PREFER_TT, &ulFileLenght);

   if(pMidi!=NULL){

       // check MUS file
       MUSheader_t *pMusHeader=(MUSheader_t *)pMidi;

       if(((pMusHeader->ID)>>8)==MUS_ID){
           // convert it to midi format
           amTrace((const U8*)"Converting MUS -> MID ...\n");
           U8 *pOut=0;
           U32 len=0;

           // allocate working buffer for midi output
           pOut=amMallocEx(MIDI_OUT_TEMP, PREFER_TT);

           // set midi output name
           char tempName[128]={0};
           char *pTempPtr=0;
           strncpy(tempName,filePath,strlen(filePath));
           pTempPtr=strrchr(tempName,'.');
           memcpy(pTempPtr+1,"mid",4);
           printf("[Please Wait] [MUS->MID] Processing midi data..\n");
           Mus2Midi(pMidi,(unsigned char *)pOut,0,&len);

           /* free up buffer with loaded MUS file, we don't need it anymore */
           amFree(pMidi);
           pMidi=(void *)pOut;
       }

       U32 delta=0;

       // check mid 0, no quit
       if(((sMThd *)pMidi)->id==ID_MTHD&&((sMThd *)pMidi)->headLenght==6L&&((sMThd *)pMidi)->format==0){
           fprintf(stderr,"Midi file loaded, size: %lu bytes.\n", ulFileLenght);

           char tempName[128]={0};
           char *pTempPtr=0;
           strncpy(tempName,filePath,strlen(filePath));
           pTempPtr=strrchr(tempName,'.');
           memcpy(pTempPtr+1,"nkt",4);

           fprintf(stderr,"[ Please wait ] Converting MID to %s. Compress: %s\n",tempName,bEnableCompression?"YES":"NO");

           // convert
           iError = Midi2Nkt(pMidi,tempName, bEnableCompression);

           //TODO: add errorcheck
       }else{
           fprintf(stderr,"File is not in MIDI 0 format. Exiting... \n");
       }

       /* free up buffer with loaded midi file, we don't need it anymore */
       amFree(pMidi);

    }

  //done..
   deinitDebug();
   fprintf(stderr,"\nDone. Press any key... \n");

   getchar();
   return 0;
}


void printInfoScreen(){
    printf("\n== MID / MUS to NKT converter v.1.00 =========\n");
    printf("date: %s %s\n",__DATE__,__TIME__);
    printf("==========================================\n");
}
