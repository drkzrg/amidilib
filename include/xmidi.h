
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef AXMIDI_H
#define AXMIDI_H

#include "c_vars.h"

/* XMIDI contain midi events */
#include "midi.h"

// make ID
#define	MAKE_ID(a,b,c,d) ((uint32) (a)<<24 | (uint32) (b)<<16 | (uint32) (c)<<8 | (uint32) (d))

/* Universal IFF identifiers */
#define ID_FORM  MAKE_ID('F','O','R','M')
#define ID_LIST  MAKE_ID('L','I','S','T')
#define ID_CAT   MAKE_ID('C','A','T',' ')
#define ID_PROP  MAKE_ID('P','R','O','P')
#define ID_NULL  MAKE_ID(' ',' ',' ',' ')

/* Identifier codes for universally recognized ContextInfo nodes */
#define IFF_CI_PROPCHUNK        MAKE_ID('p','r','o','p')
#define IFF_CI_COLLECTIONCHUNK  MAKE_ID('c','o','l','l')
#define IFF_CI_ENTRYHANDLER     MAKE_ID('e','n','h','d')
#define IFF_CI_EXITHANDLER      MAKE_ID('e','x','h','d')

/* define XMIDI specific chunk defs */
#define ID_XDIR MAKE_ID('X','D','I','R')
#define ID_XMID MAKE_ID('X','M','I','D') 
#define ID_INFO MAKE_ID('I','N','F','O') 
#define ID_TIMB MAKE_ID('T','I','M','B') 
#define ID_RBRN MAKE_ID('R','B','R','N') 
#define ID_EVNT MAKE_ID('E','V','N','T')

/* XMIDI additional controllers */
#define C_CH_LOCK           0x6e        /* Channel Lock */
#define C_CH_LOCK_PROTECT   0x6f        /* Channel Lock Protect */
#define C_VOICE_PROTECT     0x70        /* Voice Protect */
#define C_TIMBRE_PROTECT    0x71        /* Timbre Protect */
#define C_PATCH_BANK_SELECT 0x72        /* Patch Bank Select */
#define C_IND_CTRL_PREFIX   0x73        /* Indirect Controller Prefix */
#define C_FOR_LOOP          0x74        /* For Loop Controller */
#define C_NEXT              0x75        /* Next/Break Loop Controller */
#define C_CLEAR_BAR_COUNT   0x76        /* Clear Beat/Bar Count */
#define C_CALL_TRIGGER      0x77        /* Callback Trigger */
#define C_SEQ_BRA_IDX       0x78        /* Sequence Branch Index */

typedef int8 IFF_ID[4];

typedef struct IFFCHUNK {
	IFF_ID id;
	int32 size; 	
	uint8 data[]; 	
} sIffChunk;

#endif
