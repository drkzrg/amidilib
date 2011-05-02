
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/fmio.h"
#include <mint/ostruct.h>


static U32 g_save_ssp;
static long   ssp = 0;
static S16 g_lastGDOSerror=0;

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


const U8 *getLastGemdosError(void)
{
    return((const U8 *)g_arGEMDOSerror[g_lastGDOSerror]);
}

const U8 *getGemdosError(S16 iErr)
{
    switch(iErr)
    {
        case GDOS_OK:
            g_lastGDOSerror=GDOS_OK;
            return((const U8 *)g_arGEMDOSerror[0]);
        break;
        case GDOS_ERR:
            g_lastGDOSerror=GDOS_ERR;
            return((const U8 *)g_arGEMDOSerror[1]);
        break; /*Error*/
        case GDOS_DRIVE_NOT_READY:
            g_lastGDOSerror=GDOS_DRIVE_NOT_READY;
            return((const U8 *)g_arGEMDOSerror[2]);
        break;  /*Drive not ready*/
        case GDOS_UNKNOWN_COMMAND:
            g_lastGDOSerror=GDOS_UNKNOWN_COMMAND;
            return((const U8 *)g_arGEMDOSerror[3]);
        break;  /*Unknown command*/
        case GDOS_CRC_ERROR:
            g_lastGDOSerror=GDOS_CRC_ERROR;
            return((const U8 *)g_arGEMDOSerror[4]);
        break;  /*CRC (checksum) error*/
        case GDOS_BAD_REQ:
            g_lastGDOSerror=GDOS_BAD_REQ;
            return((const U8 *)g_arGEMDOSerror[5]);
        break;  /*Bad request*/
        case GDOS_SEEK_ERROR:
            g_lastGDOSerror=GDOS_SEEK_ERROR;
            return((const U8 *)g_arGEMDOSerror[6]);
        break;  /*Seek error*/
        case GDOS_UNKNOWN_MEDIA:
            g_lastGDOSerror=GDOS_UNKNOWN_MEDIA;
            return((const U8 *)g_arGEMDOSerror[7]);
        break;  /*Unknown media*/
        case GDOS_SECTOR_NOT_FOUND:
            g_lastGDOSerror=GDOS_SECTOR_NOT_FOUND;
            return((const U8 *)g_arGEMDOSerror[8]);
        break;  /*Sector not found*/
        case GDOS_OUT_OF_PAPER:
            g_lastGDOSerror=GDOS_OUT_OF_PAPER;
            return((const U8 *)g_arGEMDOSerror[9]);
        break;  /*Out of paper*/
        case GDOS_WRITE_FAULT:
            g_lastGDOSerror=GDOS_WRITE_FAULT;
            return((const U8 *)g_arGEMDOSerror[10]);
        break;  /*Write fault*/
        case GDOS_READ_FAULT:
            g_lastGDOSerror=GDOS_READ_FAULT;
            return((const U8 *)g_arGEMDOSerror[11]);
        break;  /*Read fault*/
        case GDOS_NVM_CHESUM_ERR:
            g_lastGDOSerror=GDOS_NVM_CHESUM_ERR;
            return((const U8 *)g_arGEMDOSerror[12]);
        break;  /*NVM checksum error*/
        case GDOS_WRITE_PROTECTED_MEDIA:
            g_lastGDOSerror=GDOS_WRITE_PROTECTED_MEDIA;
            return((const U8 *)g_arGEMDOSerror[13]);
        break;  /*Write protected media*/
        case GDOS_MEDIA_CHANGE_DETECTED:
            g_lastGDOSerror=GDOS_MEDIA_CHANGE_DETECTED;
            return((const U8 *)g_arGEMDOSerror[14]);
        break;  /*Media change detected*/
        case GDOS_UNKNOWN_DEVICE:
            g_lastGDOSerror=GDOS_UNKNOWN_DEVICE;
            return((const U8 *)g_arGEMDOSerror[15]);
        break;  /*Unknown device*/
        case GDOS_BAD_SECTOR_ON_FORMAT:
            g_lastGDOSerror=GDOS_BAD_SECTOR_ON_FORMAT;
            return((const U8 *)g_arGEMDOSerror[16]);
        break;  /*Bad sectors on format*/
        case GDOS_INSERT_OTHER_DISK_REQ:
            g_lastGDOSerror=GDOS_INSERT_OTHER_DISK_REQ;
            return((const U8 *)g_arGEMDOSerror[17]);
        break;  /*Insert other disk (request)*/
        case GDOS_INVALID_GDOS_FUNC_NB:
            g_lastGDOSerror=GDOS_INVALID_GDOS_FUNC_NB;
            return((const U8 *)g_arGEMDOSerror[18]);
        break;  /*Invalid GEMDOS function number*/
        case GDOS_FILE_NOT_FOUND:
            g_lastGDOSerror=GDOS_FILE_NOT_FOUND;
            return((const U8 *)g_arGEMDOSerror[19]);
        break;  /*File not found*/
        case GDOS_PATH_NOT_FOUND:
            g_lastGDOSerror=GDOS_PATH_NOT_FOUND;
            return((const U8 *)g_arGEMDOSerror[20]);
        break;  /*Path not found*/
        case GDOS_HANDLE_POOL_EXHAUSTED:
            g_lastGDOSerror=GDOS_HANDLE_POOL_EXHAUSTED;
            return((const U8 *)g_arGEMDOSerror[21]);
        break;  /*Handle pool exhausted*/
        case GDOS_ACCESS_DENIED:
            g_lastGDOSerror=GDOS_ACCESS_DENIED;
            return((const U8 *)g_arGEMDOSerror[22]);
        break;  /*Access denied*/
        case GDOS_INVALID_HANDLE:
            g_lastGDOSerror=GDOS_INVALID_HANDLE;
            return((const U8 *)g_arGEMDOSerror[23]);
        break;  /*Invalid handle*/
        case GDOS_INSUFFICIENT_MEMORY:
            g_lastGDOSerror=GDOS_INSUFFICIENT_MEMORY;
            return((const U8 *)g_arGEMDOSerror[24]);
        break;  /*Insufficient memory*/
        case GDOS_INVALID_MEM_BLOCK_ADDR:
            g_lastGDOSerror=GDOS_INVALID_MEM_BLOCK_ADDR;
            return((const U8 *)g_arGEMDOSerror[25]);
        break;  /*Invalid memory block address*/
        case GDOS_INVALID_DRIVE_SPEC:
            g_lastGDOSerror=GDOS_INVALID_DRIVE_SPEC;
            return((const U8 *)g_arGEMDOSerror[26]);
        break;  /*Invalid drive specification*/
        case GDOS_NOT_THE_SAME_DRIVE:
            g_lastGDOSerror=GDOS_NOT_THE_SAME_DRIVE;
            return((const U8 *)g_arGEMDOSerror[27]);
        break;  /*Not the same drive*/
        case GDOS_NO_MORE_FILES:
            g_lastGDOSerror=GDOS_NO_MORE_FILES;
            return((const U8 *)g_arGEMDOSerror[28]);
        break;  /*No more files*/
        case GDOS_RECORD_LOCKED:
            g_lastGDOSerror=GDOS_RECORD_LOCKED;
            return((const U8 *)g_arGEMDOSerror[29]);
        break;  /*Record is already locked*/
        case GDOS_INVALID_LOCK_REMOVAL_REQ:
            g_lastGDOSerror=GDOS_INVALID_LOCK_REMOVAL_REQ;
            return((const U8 *)g_arGEMDOSerror[30]);
        break;  /*Invalid lock removal request*/
        case GDOS_ARG_RANGE_ERR:
            g_lastGDOSerror=GDOS_ARG_RANGE_ERR;
            return((const U8 *)g_arGEMDOSerror[31]);
		break;  /*Argument range error*/
        case GDOS_INTERNAL_ERR:
            g_lastGDOSerror=GDOS_INTERNAL_ERR;
            return((const U8 *)g_arGEMDOSerror[32]);
        break;  /*GEMDOS internal error*/
        case GDOS_INVALID_EXE_FMT:
            g_lastGDOSerror=GDOS_INVALID_EXE_FMT;
            return((const U8 *)g_arGEMDOSerror[33]);
        break;  /*Invalid executable file format*/
        case GDOS_MEM_BLOCK_GROWTH_FAIL:
            g_lastGDOSerror=GDOS_MEM_BLOCK_GROWTH_FAIL;
            return((const U8 *)g_arGEMDOSerror[34]);
        break;  /*Memory block growth failure*/
        case GDOS_TOO_MANY_SYMLINKS:
            g_lastGDOSerror=GDOS_TOO_MANY_SYMLINKS;
            return((const U8 *)g_arGEMDOSerror[35]);
        break;  /*Too many symbolic links*/
        case GDOS_MOUNT_POINT_CROSSED:
            g_lastGDOSerror=GDOS_MOUNT_POINT_CROSSED;
            return((const U8 *)g_arGEMDOSerror[36]);
        break;  /*Mount point crossed*/
        default:
            /* unknown error */
            g_lastGDOSerror=GDOS_ERR;
            return((const U8 *)g_arGEMDOSerror[1]);
        break;
    }
 /* to make compiler happy ..*/
 return((const U8 *)g_arGEMDOSerror[1]);
}

/* loads file to specified type memory */
void *loadFile(U8 *szFileName, eMemoryFlag memFlag,  U32 *fileLenght){
S32 fileHandle;
_DTA *pDTA=NULL;
void *pData=NULL;
S16 iRet=0;
U32 lRet=0L;

    fileHandle = Fopen( szFileName, FO_READ );
    *fileLenght=0L;

    if((fileHandle)>0L){
    pDTA=Fgetdta();
    iRet=Fsfirst( szFileName, 0 );

    if(iRet==0){
    /* file found */
	 
    *fileLenght=pDTA->dta_size;
    /* allocate buffer */
    pData=(void *)Mxalloc( (long)(*fileLenght)+1, memFlag);

    if(pData!=NULL){
      lRet=Fread( (int)fileHandle, (long)(*fileLenght), pData );

      /* not all data being read */
      if(lRet!=(*fileLenght)){
	/* so we have error, free up memory */
	Mfree(pData);
	pData=NULL;
      }
	
        Fclose((int)fileHandle);
        return (pData);
     }
     else{
      /*no memory available */
      return NULL;
     }
    }
		else
		{
		 Fclose((int)fileHandle);

		 /* file not found */
		 getGemdosError(iRet);
         return NULL;
        }
    }
    else{
        /* print GEMDOS error code */
        getGemdosError((S16)fileHandle);
        return NULL;
    }
}

U32 getFreeMem(eMemoryFlag memFlag){
    void *pMem=(void *)Mxalloc( -1L, memFlag);
    return((U32)pMem);
}


void am_setSuperOn(void) {
   ssp = (long)Super((void*)0);         /* enter supervisor mode */
}

void am_setSuperOff(void) {
	SuperToUser(ssp); /* return processor to user mode */
}
