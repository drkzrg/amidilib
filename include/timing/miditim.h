#ifndef __MIDITIM_H__
#define __MIDITIM_H__

#include "c_vars.h"
#include "amlog.h"

/** Calculate timestep function
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/

static inline U32 am_calculateTimeStep(const U16 qpm,const U16 ppq,const U16 ups){
    static U32 ppu=0;
    static U32 temp=0;

    temp=(U32)qpm*(U32)ppq;

    if(temp<0x10000){
        ppu=((temp*0x10000)/60)/(U32)ups;
    }else{
        ppu=((temp/60)*0x10000)/(U32)ups;
    }

 return ppu;
}

/** Calculate timestep function (float version)
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/
float am_calculateTimeStepFlt(const U16 qpm,const U16 ppq,const U16 ups);

/** Simple time division to console function
*   with example of handling timeDivision data
*   @param timeDivision - time division value from midi file header
*   @return decoded time division in PPQN or FPS
*/

U16 am_decodeTimeDivisionInfo(U16 timeDivision);

/******************** time measuring functions */

/** get current time signature  
* @return none
*/

void getTimeStamp();

/** Utility function for measuring delta time  
 *   @return delta time since last getTimeStamp() function call in ms
 */
U32 getTimeDelta();

#define DEFAULT_MPQN 500000	   	// microsecs per quaternote
#define DEFAULT_PPQN 96	   		// default pulses per quater note (resolution)
#define SEQUENCER_UPDATE_HZ 200 // sequencer update rate
#define DEFAULT_BPM 120	   		// 60,000,000/DEFAULT_MPQN quaternotes per minute


#endif

