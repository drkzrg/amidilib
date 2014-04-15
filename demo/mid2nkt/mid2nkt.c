
#include <stdio.h>
#include <string.h>

#include "midi2nkt.h"       // mid2nkt conversion
#include "timing/miditim.h" // time measurement
#include "fmio.h"           // disc i/o
#include "amlog.h"          // logging
#include "midi.h"           // midi

#include "lzo/minilzo.h" //compression

void printInfoScreen();

int main(int argc, char *argv[]){

    initDebug("mid2nkt.log");

    //todo check parameters for compression

    if(argc>=1&&argv[1]!='\0'){
         fprintf(stderr,"Trying to load %s\n",argv[1]);
     }else{
         fprintf(stderr,"No specified mid filename! exiting\n");
         return 0;
     }


  // load midi file into memory
  U32 iError=0;
  U32 ulFileLenght=0L;
  void *pMidi=0;

  pMidi=loadFile((U8 *)argv[1], PREFER_TT, &ulFileLenght);

   if(pMidi!=NULL){
       // check mid 0, no quit
       fprintf(stderr,"Midi file loaded, size: %u bytes.\n",(unsigned int) ulFileLenght);

       U32 time=0,delta=0;
       sMThd *pMidiInfo=0;

       if(((sMThd *)pMidi)->id==ID_MTHD&&((sMThd *)pMidi)->headLenght==6L&&((sMThd *)pMidi)->format==0){
           char tempName[128]={0};
           char *pTempPtr=0;
           strncpy(tempName,argv[1],strlen(argv[1]));
           pTempPtr=strrchr(tempName,'.');
           memcpy(pTempPtr+1,"nkt",4);

           printInfoScreen();
            fprintf(stderr,"[ Please wait ] Converting MID to %s.\n",tempName);
           // convert
           time = getTimeStamp();
           iError = Midi2Nkt(pMidi,tempName,FALSE);
           delta=getTimeDelta();

           if(iError==0){
            fprintf(stderr,"MIDI file conversion in ~%4ul[sec]/~%4ul[min]\n", delta, delta/60);
           }

       }else{
            fprintf(stderr,"File is not in MIDI 0 format. Exiting... \n");
       }

       /* free up buffer with loaded midi file, we don't need it anymore */
       amFree(&pMidi);

    }

  //done..
   deinitDebug();

   return 0;
}


void printInfoScreen(){
    printf("\n== MID to NKT converter v.1.00 =========\n");
    printf("date: %s %s\n",__DATE__,__TIME__);
    printf("==========================================\n");
}
