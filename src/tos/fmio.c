
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <mint/ostruct.h>
#include <mint/osbind.h>
#include "memory/memory.h"
#include "fmio.h"

#include <stdio.h>

static int16 g_lastGDOSerror=0;

static const char *g_arGEMDOSerror[71]= { 
  "No error.",\
  "Error.",\
  "Drive not ready.",\
  "Unknown command.",\
  "CRC (checksum) error.",\
  "Bad request.",\
  "Seek error.",\
  "Unknown media.",\
  "Sector not found.",\
  "Out of paper.",\
  "Write fault.",\
  "Read fault.",\
  "NVM checksum error.",\
  "Write protected media.",\
  "Media change detected.",\
  "Unknown device.",\
  "Bad sectors on format.",\
  "Insert other disk (request).",\
  "Invalid GEMDOS function number.",\
  "File not found.",\
  "Path not found.",\
  "Handle pool exhausted.",\
  "Access denied.",\
  "Invalid handle.",\
  "Insufficient memory.",\
  "Invalid memory block address.",\
  "Invalid drive specification.",\
  "Not the same drive.",\
  "No more files.",\
  "Record is already locked.",\
  "Invalid lock removal request.",\
  "Argument range error.",\
  "GEMDOS internal error.",\
  "Invalid executable file format.",\
  "Memory block growth failure.",\
  "Too many symbolic links.",\
  "Mount point crossed."
};


const uint8 *getLastGemdosError(){
    return getGemdosError(g_lastGDOSerror);
}

const uint8 *getGemdosError(const int16 iErr){

uint16 idx=1;

    switch(iErr){
        case GDOS_OK:{
            g_lastGDOSerror = GDOS_OK;
            idx=0;
        } break;
        case GDOS_ERR:{
            g_lastGDOSerror = GDOS_ERR;
            idx=1;
        } break; /*Error*/
        case GDOS_DRIVE_NOT_READY:{
            g_lastGDOSerror = GDOS_DRIVE_NOT_READY;
            idx=2;
        } break;  /*Drive not ready*/
        case GDOS_UNKNOWN_COMMAND:{
            g_lastGDOSerror = GDOS_UNKNOWN_COMMAND;
            idx=3;
        } break;  /*Unknown command*/
        case GDOS_CRC_ERROR:
            g_lastGDOSerror = GDOS_CRC_ERROR;
            idx=4;
        break;  /*CRC (checksum) error*/
        case GDOS_BAD_REQ:
            g_lastGDOSerror=GDOS_BAD_REQ;
            idx=5;
        break;  /*Bad request*/
        case GDOS_SEEK_ERROR:
            g_lastGDOSerror=GDOS_SEEK_ERROR;
            idx=6;
        break;  /*Seek error*/
        case GDOS_UNKNOWN_MEDIA:
            g_lastGDOSerror=GDOS_UNKNOWN_MEDIA;
            idx=7;
        break;  /*Unknown media*/
        case GDOS_SECTOR_NOT_FOUND:
            g_lastGDOSerror=GDOS_SECTOR_NOT_FOUND;
            idx=8;
        break;  /*Sector not found*/
        case GDOS_OUT_OF_PAPER:
            g_lastGDOSerror=GDOS_OUT_OF_PAPER;
            idx=9;
        break;  /*Out of paper*/
        case GDOS_WRITE_FAULT:
            g_lastGDOSerror=GDOS_WRITE_FAULT;
            idx=10;
        break;  /*Write fault*/
        case GDOS_READ_FAULT:
            g_lastGDOSerror=GDOS_READ_FAULT;
            idx=11;
        break;  /*Read fault*/
        case GDOS_NVM_CHESUM_ERR:
            g_lastGDOSerror=GDOS_NVM_CHESUM_ERR;
            idx=12;
        break;  /*NVM checksum error*/
        case GDOS_WRITE_PROTECTED_MEDIA:
            g_lastGDOSerror=GDOS_WRITE_PROTECTED_MEDIA;
            idx=13;
        break;  /*Write protected media*/
        case GDOS_MEDIA_CHANGE_DETECTED:
            g_lastGDOSerror=GDOS_MEDIA_CHANGE_DETECTED;
            idx=14;
        break;  /*Media change detected*/
        case GDOS_UNKNOWN_DEVICE:
            g_lastGDOSerror=GDOS_UNKNOWN_DEVICE;
            idx=15;
        break;  /*Unknown device*/
        case GDOS_BAD_SECTOR_ON_FORMAT:
            g_lastGDOSerror=GDOS_BAD_SECTOR_ON_FORMAT;
            idx=16;
        break;  /*Bad sectors on format*/
        case GDOS_INSERT_OTHER_DISK_REQ:
            g_lastGDOSerror=GDOS_INSERT_OTHER_DISK_REQ;
            idx=17;
        break;  /*Insert other disk (request)*/
        case GDOS_INVALID_GDOS_FUNC_NB:
            g_lastGDOSerror=GDOS_INVALID_GDOS_FUNC_NB;
            idx=18;
        break;  /*Invalid GEMDOS function number*/
        case GDOS_FILE_NOT_FOUND:
            g_lastGDOSerror=GDOS_FILE_NOT_FOUND;
            idx=19;
        break;  /*File not found*/
        case GDOS_PATH_NOT_FOUND:
            g_lastGDOSerror=GDOS_PATH_NOT_FOUND;
            idx=20;
        break;  /*Path not found*/
        case GDOS_HANDLE_POOL_EXHAUSTED:
            g_lastGDOSerror=GDOS_HANDLE_POOL_EXHAUSTED;
            idx=21;
        break;  /*Handle pool exhausted*/
        case GDOS_ACCESS_DENIED:
            g_lastGDOSerror=GDOS_ACCESS_DENIED;
            idx=22;
        break;  /*Access denied*/
        case GDOS_INVALID_HANDLE:
            g_lastGDOSerror=GDOS_INVALID_HANDLE;
            idx=23;
        break;  /*Invalid handle*/
        case GDOS_INSUFFICIENT_MEMORY:
            g_lastGDOSerror=GDOS_INSUFFICIENT_MEMORY;
            idx=24;
        break;  /*Insufficient memory*/
        case GDOS_INVALID_MEM_BLOCK_ADDR:
            g_lastGDOSerror=GDOS_INVALID_MEM_BLOCK_ADDR;
            idx=25;
        break;  /*Invalid memory block address*/
        case GDOS_INVALID_DRIVE_SPEC:
            g_lastGDOSerror=GDOS_INVALID_DRIVE_SPEC;
            idx=26;
        break;  /*Invalid drive specification*/
        case GDOS_NOT_THE_SAME_DRIVE:
            g_lastGDOSerror=GDOS_NOT_THE_SAME_DRIVE;
            idx=27;
        break;  /*Not the same drive*/
        case GDOS_NO_MORE_FILES:
            g_lastGDOSerror=GDOS_NO_MORE_FILES;
            idx=28;
        break;  /*No more files*/
        case GDOS_RECORD_LOCKED:
            g_lastGDOSerror=GDOS_RECORD_LOCKED;
            idx=29;
        break;  /*Record is already locked*/
        case GDOS_INVALID_LOCK_REMOVAL_REQ:
            g_lastGDOSerror=GDOS_INVALID_LOCK_REMOVAL_REQ;
            idx=30;
        break;  /*Invalid lock removal request*/
        case GDOS_ARG_RANGE_ERR:
            g_lastGDOSerror=GDOS_ARG_RANGE_ERR;
            idx=31;
		break;  /*Argument range error*/
        case GDOS_INTERNAL_ERR:
            g_lastGDOSerror=GDOS_INTERNAL_ERR;
            idx=32;
        break;  /*GEMDOS internal error*/
        case GDOS_INVALID_EXE_FMT:
            g_lastGDOSerror=GDOS_INVALID_EXE_FMT;
            idx=33;
        break;  /*Invalid executable file format*/
        case GDOS_MEM_BLOCK_GROWTH_FAIL:
            g_lastGDOSerror=GDOS_MEM_BLOCK_GROWTH_FAIL;
            idx=34;
        break;  /*Memory block growth failure*/
        case GDOS_TOO_MANY_SYMLINKS:
            g_lastGDOSerror=GDOS_TOO_MANY_SYMLINKS;
            idx=35;
        break;  /*Too many symbolic links*/
        case GDOS_MOUNT_POINT_CROSSED:
            g_lastGDOSerror=GDOS_MOUNT_POINT_CROSSED;
            idx=36;
        break;  /*Mount point crossed*/
        default:
            /* unknown error */
            g_lastGDOSerror=GDOS_ERR;
            idx=1;
        break;
    }

 return ((const uint8*)g_arGEMDOSerror[idx]);
}

/* loads file to specified type memory */
void *loadFile(const uint8 *szFileName, eMemoryFlag memFlag,  uint32 *fileLenght){

    amTrace("[GEMDOS] load file: %s\n",szFileName);

    int16 fileHandle=GDOS_INVALID_HANDLE;
    _DTA *pDTA=NULL;

    fileHandle = Fopen( szFileName, S_READWRITE );
    *fileLenght=0L;

    if((fileHandle)>0){
	int16 iRet=0;
	
    pDTA=Fgetdta();
    iRet=Fsfirst( szFileName, 0 );

    if(iRet==0){

    /* file found */
    void *pData=NULL;
	 
    *fileLenght=pDTA->dta_size;

    /* allocate buffer */
     pData=(void *)amMallocEx((MemSize)(*fileLenght)+1,memFlag);
     
     if(pData!=NULL){
      int32 lRet=0L;
      amMemSet(pData,0,(*fileLenght)+1);

      lRet=Fread(fileHandle, (*fileLenght), pData );

      if(lRet<0){
            //GEMDOS ERROR TODO, display error for now
            amTrace("[GEMDOS] Error: %s\n",getGemdosError((int16)lRet));
      }else{
          /* not all data being read */
          if(lRet!=(*fileLenght)){
            printf((const char *)"Fatal error, unexpected end of file.\n");
            amTrace("[GEMDOS] Read error. Unexpected EOF.\n");

            /* so we have error, free up memory */
            amFree(pData);
          }

      }

      amTrace("[GEMDOS] Closing file handle : [%d] \n", fileHandle);

      int16 err=Fclose(fileHandle);

      if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] \n", fileHandle, getGemdosError(err));
      }

      return (pData);

     } else {
      /*no memory available */
      printf("Not enough memory to load file.\n");
      amTrace("Not enough memory to load file.\n");
      return NULL;
     }

    }else{
      amTrace("[GEMDOS] Closing file handle : [%d] \n", fileHandle);

      int16 err=Fclose(fileHandle);

      if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] \n", fileHandle, getGemdosError(err));
      }

      /* file not found */

      printf("[GEMDOS] Error: %s\n", getGemdosError(fileHandle));
      amTrace("[GEMDOS] Error: %s \n", getGemdosError(fileHandle));
      return NULL;
     }
    }else{
        /* print GEMDOS error code */
        printf("[GEMDOS] Error: %s \n", getGemdosError(fileHandle));
        amTrace("[GEMDOS] Error: %s \n",getGemdosError(fileHandle));
        return NULL;
    }
}

int32 saveFile(const uint8 *szFileName, const void *memBlock, const uint32 memBlockSize){

  int16 fileHandle = Fcreate( szFileName, 0 );
    
    if(fileHandle>0){
	 int32 iRet=0;
     amTrace("[GEMDOS] Create file, gemdos handle: %d\n",fileHandle);

     iRet=Fwrite(fileHandle,memBlockSize,memBlock);
       
     if(iRet==memBlockSize){
        printf("\n%s saved [%ld bytes written].\n",szFileName,iRet);
      }else{
        /* print GEMDOS error code */
        printf("[GEMDOS] Error: %s\n", getGemdosError(fileHandle));
      }

     amTrace("[GEMDOS] Closing file handle : [%d] \n", fileHandle);

      int16 err=Fclose(fileHandle);

      if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] \n", fileHandle, getGemdosError(err));
      }

      return 0L;   
    }else{
       /* print GEMDOS error code */
        printf("[GEMDOS] Error: %s\n", getGemdosError((int16)fileHandle));
	     return -1L;  
    }
 
}
