
#if AMIDILIB_USE_LIBC
#include <string.h>
#else
#include "amstring.h"
#endif

#include "dmus.h"           // MUS->MID conversion
#include "midi.h"           // midi
#include "midi2nkt.h"

#ifdef ENABLE_GEMDOS_IO
#include "gemdosio.h"       // disc i/o
#else
#include <stdio.h>
#endif

#include "core/amprintf.h"
#include "core/logger.h"    // logging
#include "core/machine.h"

#include <osbind.h>

static const uint32 MIDI_OUT_TEMP = 100*1024; // temporary buffer for MUS->MID conversion
static const uint32 MAX_GEMDOS_FILEPATH = 128;

void printInfoScreen(void);

int main(int argc, char *argv[])
{

Bool bEnableCompression=FALSE;
uint8 *filePath=0;

  Supexec(checkMachine);
  amSetDefaultUserMemoryCallbacks();

#ifdef ENABLE_GEMDOS_IO
  initGemdos();
#endif

  initLogger("mid2nkt.log");

  printInfoScreen();

  // check parameters for compression
  if( ((argc==2) && (strlen(argv[1])!=0)))
  {
      amPrintf("Trying to load %s"NL,argv[1]);
      filePath = argv[1];

  } 
  else if((argc==3) && (strlen(argv[1])!=0)&&(strlen(argv[2])!=0))
  {

      if( ( (strcmp("-c",argv[1])==0) || (strcmp("--compress", argv[1])==0) ))
      {
       bEnableCompression = TRUE;
      }

      amPrintf("Trying to load %s"NL,argv[2]);
      filePath=argv[2];

  }
  else
  {
      amPrintf("No specified mid / mus filename or bad parameters! Exiting ..."NL);
      deinitLogger();

#if AMIDILIB_USE_LIBC
#else   
   (void)Cconin();
#endif

      return 0;
  }

  // load midi file into memory
  uint32 ulFileLenght=0;
  
  void *pMidi = loadFile(filePath, MF_PREFER_FASTRAM, &ulFileLenght,FILE_RO);

   if(pMidi!=NULL)
   {
        char tempName[MAX_GEMDOS_FILEPATH];

       // check MUS file
       MUSheader_t *pMusHeader=(MUSheader_t *)pMidi;

       if(((pMusHeader->ID)>>8)==MUS_ID)
       {
           // convert it to midi format
           amTrace("Converting MUS -> MID ..."NL,0);

           uint32 len=0;

           // allocate working buffer for midi output
           uint8 *pOut= (uint8 *)amMalloc(MIDI_OUT_TEMP, MF_PREFER_FASTRAM,NULL);
           amMemSet(tempName,0,MAX_GEMDOS_FILEPATH);

           // set midi output name
           strncpy(tempName,filePath,MAX_GEMDOS_FILEPATH-1);

           uint8 *fileExtPtr = strrchr(tempName,(int)'.');

           if(fileExtPtr!=NULL)
           {
               amMemCpy(fileExtPtr+1,"mid",4);
               amPrintf("[Please Wait] [MUS->MID] Processing midi data.."NL);
               int32 ret = Mus2Midi(pMidi,(unsigned char *)pOut,0,&len);
           } 
           else 
           {
               amPrintf("[Error] Filename update failed."NL);

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
       if(((sMThd *)pMidi)->id==ID_MTHD && ((sMThd *)pMidi)->headLenght==6L&& (((sMThd *)pMidi)->format==0||((sMThd *)pMidi)->format==1))
       {
          amPrintf("Midi file loaded, size: %u bytes."NL, ulFileLenght);
          
          amMemSet(tempName,0,MAX_GEMDOS_FILEPATH-1);
          strncpy(tempName,filePath,MAX_GEMDOS_FILEPATH-1);

          uint8 *fileExtPtr = strrchr(tempName,'.');

          if(fileExtPtr!=NULL)
          {
           amMemCpy(fileExtPtr+1,"nkt",4);
           amPrintf("[ Please wait ] Converting MIDI %d to %s. Compress: %s"NL,((sMThd *)pMidi)->format, tempName, bEnableCompression?"YES":"NO");

           // convert
           sNktSeq* pSeq = Midi2Nkt(pMidi,tempName, bEnableCompression);

           if(pSeq)
           {
                // release sequence
                destroyNktSequence(pSeq);
           }
           else
           {
               amPrintf("[MID->NKT] conversion error. Exiting."NL);
           }
          }
          else
          {
               amPrintf("[Error] Output filename update failed."NL);
          }


       }else{
           amPrintf("File is not in MIDI 0 or 1 format. Exiting... "NL);
       }

       /* free up buffer with loaded midi file, we don't need it anymore */
       amFree(pMidi,0);

    }

  //done..
   deinitLogger();

#ifdef ENABLE_GEMDOS_IO
   deinitGemdos();
#endif

   amPrintf(NL "Done. Press any key... "NL);

#if AMIDILIB_USE_LIBC
#else   
   (void)Cconin();
#endif

   return 0;
}

void printInfoScreen(void)
{
    amPrintf(NL "== MID / MUS to NKT converter v.1.4 ========="NL);
 #if AMIDILIB_USE_LIBC
    amPrintf("build date: %s %s"NL,__DATE__,__TIME__);
 #else  
    amPrintf("build date: %s %s nolibc"NL,__DATE__,__TIME__);
 #endif
    amPrintf("(c)Nokturnal 2007-22"NL);   
    amPrintf("=========================================="NL);
}
