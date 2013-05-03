
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


/* ###################################################################################
#  INCLUDES
################################################################################### */

#include <assert.h>
#include "c_vars.h"

/* Compiler sanity check */

void compilerSanityCheck(void)
{
 /* check compiler sanity */
 /* if something below will be not right then compiler will moan ;)) ..*/
 /* In this case check your compiler docs with typedefs from C_VARS.H */

 assert(sizeof(U8)==1);
    assert(sizeof(U16)==2);
    assert(((U16)-1)>0);
    assert(sizeof(U32)==4);    
    assert((U32)-1>0);    
    assert(sizeof(S32)==4);    
    assert((S32)-1==-1);    
    assert(sizeof(float)==4);
    assert(sizeof(double)==10);
    assert(sizeof(long double)==10);

}

// check processor endianess ( processor sanity check ;) )
// returns:
// TRUE - for little endian, x86, PS2
//FALSE / for big endian - // Big Endian - GameCube, Atari

BOOL checkEndianess()
{
  U16 uint16;
  uint16=1;

 if (*(U8 *)&uint16==1){
    //little endian - x86, PS2
   return TRUE;
 } else{
  // Big Endian - GameCube, Atari
  return FALSE;
 }
}

