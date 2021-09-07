
#include <stdio.h>
#include <string.h>

#include "dmus.h"           // MUS->MID conversion
#include "fmio.h"           // disc i/o
#include "amlog.h"          // logging
#include "midi.h"           // midi
#include "midi2nkt.h"
#include <osbind.h>

static const uint32 MIDI_OUT_TEMP = 100*1024; // temporary buffer for MUS->MID conversion
static const uint32 MAX_GEMDOS_FILEPATH = 128;

void printInfoScreen(void);

int main(int argc, char *argv[])
{

Bool bEnableCompression=FALSE;
uint8 *filePath=0;

  amSetDefaultUserMemoryCallbacks();

  initDebug("MID2NKT.LOG");

  printInfoScreen();

  // check parameters for compression
  if( ((argc==2) && (strlen(argv[1])!=0))){
      printf("Trying to load %s\n",argv[1]);
      filePath = argv[1];

  } else if((argc==3) && (strlen(argv[1])!=0)&&(strlen(argv[2])!=0)){

      if( ( (strcmp("-c",argv[1])==0) || (strcmp("--compress", argv[1])==0) )){
       bEnableCompression = TRUE;
      }

      printf("Trying to load %s\n",argv[2]);
      filePath=argv[2];

  }else{
      printf("No specified mid / mus filename or bad parameters! Exiting ...\n");
      deinitDebug();
      (void)Cconin();
      return 0;
  }

  // load midi file into memory
  uint32 iError=0;
  uint32 ulFileLenght=0L;
  void *pMidi=0;
  char tempName[MAX_GEMDOS_FILEPATH];

  pMidi = loadFile(filePath, PREFER_TT, &ulFileLenght);

   if(pMidi!=NULL)
   {
       // check MUS file
       MUSheader_t *pMusHeader=(MUSheader_t *)pMidi;

       if(((pMusHeader->ID)>>8)==MUS_ID){
           // convert it to midi format
           amTrace((const uint8*)"Converting MUS -> MID ...\n");

           uint8 *pOut=0;
           uint32 len=0;

           // allocate working buffer for midi output
           pOut = (uint8 *)amMalloc(MIDI_OUT_TEMP, PREFER_TT,NULL);
           amMemSet(tempName,0,MAX_GEMDOS_FILEPATH);

           // set midi output name
           uint8 *pTempPtr=0;
           strncpy(tempName,filePath,MAX_GEMDOS_FILEPATH-1);

           pTempPtr = strrchr(tempName,'.');

           if(pTempPtr){

               amMemCpy(pTempPtr+1,"mid",4);
               printf("[Please Wait] [MUS->MID] Processing midi data..\n");
               int32 ret = Mus2Midi(pMidi,(unsigned char *)pOut,0,&len);

           } else {

               printf("[Error] Filename update failed.\n");

               /* free up buffer and quit */
               amFree(pMidi,0);
               return 0;
           }

           /* free up buffer with loaded MUS file, we don't need it anymore */
           amFree(pMidi,0);
           pMidi=(void *)pOut;
       }

       uint32 delta=0;

       // check mid 0,1 no quit
       if(((sMThd *)pMidi)->id==ID_MTHD && ((sMThd *)pMidi)->headLenght==6L&& (((sMThd *)pMidi)->format==0||((sMThd *)pMidi)->format==1)){

           printf("Midi file loaded, size: %u bytes.\n", ulFileLenght);

           uint8 *pTempPtr=0;

           amMemSet(tempName,0,MAX_GEMDOS_FILEPATH);
           strncpy(tempName,filePath,MAX_GEMDOS_FILEPATH-1);

           pTempPtr=strrchr(tempName,'.');
           amMemCpy(pTempPtr+1,"nkt",4);

           printf("[ Please wait ] Converting MIDI %d to %s. Compress: %s\n",((sMThd *)pMidi)->format, tempName, bEnableCompression?"YES":"NO");

           // convert
           sNktSeq* pSeq = Midi2Nkt(pMidi,tempName, bEnableCompression);

           if(pSeq){
                // release sequence
                destroyNktSequence(pSeq);
           }else{
               printf("[MID->NKT] conversion error. Exiting.\n");
           }

       }else{
           printf("File is not in MIDI 0 or 1 format. Exiting... \n");
       }

       /* free up buffer with loaded midi file, we don't need it anymore */
       amFree(pMidi,0);

    }

  //done..
   deinitDebug();
   printf("\nDone. Press any key... \n");

   (void)Cconin();

   return 0;
}


void printInfoScreen(void){
    printf("\n== MID / MUS to NKT converter v.1.4 =========\n");
    printf("date: %s %s\n",__DATE__,__TIME__);
    printf("==========================================\n");
}
