
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AXMIDI_H__
#define __AXMIDI_H__

/* XMIDI contain midi events */
#include "midi.h"

// make ID
#define	MAKE_ID(a,b,c,d) ((uint32_t) (a)<<24 | (uint32_t) (b)<<16 | (uint32_t) (c)<<8 | (uint32_t) (d))

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
#define ID_XDIR 0x58444952  /* XDIR */
#define ID_XMID 0x584D4944  /* XMID */
#define ID_INFO 0x494E464F  /* INFO */
#define ID_TIMB 0x54494D42  /* TIMB */
#define ID_RBRN 0x5242524E  /* RBRN */
#define ID_EVNT 0x45564E54  /* EVNT */


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

#endif
