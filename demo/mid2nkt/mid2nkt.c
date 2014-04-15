
#include <stdio.h>
#include "midi2nkt.h"

#include "timing/miditim.h"
#include "fmio.h"           // disc i/o
#include "amlog.h"
#include "midi.h"

void printInfoScreen();

int main(int argc, char *argv[]){

    if(argc>=1&&argv[1]!='\0'){
         fprintf(stderr,"Trying to load %s\n",argv[1]);
     }else{
         fprintf(stderr,"No specified mid filename! exiting\n");
         return 0;
     }


  // load midi file into memory
  U32 ulFileLenght=0L;
  void *pMidi=0;

  pMidi=loadFile((U8 *)argv[1], PREFER_TT, &ulFileLenght);

   if(pMidi!=NULL){
       // check mid 0, no quit
       fprintf(stderr,"Midi file loaded, size: %u bytes.\n",(unsigned int) ulFileLenght);


       U32 time=0,delta=0;
       time = getTimeStamp();


       delta=getTimeDelta();

       /* free up buffer with loaded midi file, we don't need it anymore */
       amFree(&pMidi);

    }


  printInfoScreen();

  // convert


  //done..
}


void printInfoScreen(){


    printf("\n== MID to NKT converter v.1.00 =========\n");
    printf("date: %s %s\n",__DATE__,__TIME__);

    printf("==========================================\n");
    printf("Ready...\n");
}
