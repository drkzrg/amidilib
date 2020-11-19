#ifndef _MIDITIM_H_
#define _MIDITIM_H_

#include "c_vars.h"

/** Calculate timestep function
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/

static inline uint32 amCalculateTimeStep(const uint16 qpm,const uint16 ppq,const uint16 ups){
    static uint32 ppu=0;
    static uint32 temp=0;

    temp=(uint32)qpm*(uint32)ppq;

    if(temp<0x10000){
        ppu=((temp*0x10000)/60)/(uint32)ups;
    }else{
        ppu=((temp/60)*0x10000)/(uint32)ups;
    }

 return ppu;
}

/** Simple time division to console function
*   with example of handling timeDivision data
*   @param timeDivision - time division value from midi file header
*   @return decoded time division in PPQN or FPS
*/

uint16 amDecodeTimeDivisionInfo(const uint16 timeDivision);

/******************** time measuring functions */

/** get current time signature  
* @return none
*/

void amGetTimeStamp(void);

/** Utility function for measuring delta time  
 *   @return delta time since last amGetTimeStamp() function call in ms
 */
uint32 amGetTimeDelta(void);

#define DEFAULT_MPQN 500000	   	// microsecs per quaternote
#define DEFAULT_PPQN 96	   		// default pulses per quater note (resolution)
#define SEQUENCER_UPDATE_HZ ((uint32)200) // sequencer update rate
#define DEFAULT_BPM 120	   		// 60,000,000/DEFAULT_MPQN quaternotes per minute


#endif

