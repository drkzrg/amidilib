#ifndef __MIDITIM_H__
#define __MIDITIM_H__

#include "c_vars.h"

/** Calculate timestep function
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/
U32 am_calculateTimeStep(U16 qpm, U16 ppq, U16 ups);

/** Calculate timestep function (float version)
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/
float am_calculateTimeStepFlt(U16 qpm, U16 ppq, U16 ups);

/** Simple time division to console function
*   with example of handling timeDivision data
*   @param timeDivision - time division value from midi file header
*   @return decoded time division in PPQN or FPS
*/

U16 am_decodeTimeDivisionInfo(U16 timeDivision);

/******************** time measuring functions */

/** get current time signature  
* @return returns current time signature in ms
*/

float getTimeStamp();

/** Utility function for measuring delta time  
 *   @return delta time since last getTimeStamp() function call in ms
 */
float getTimeDelta();

#define DEFAULT_MPQN 500000	   	// microsecs per quaternote
#define DEFAULT_PPQN 96	   		// default pulses per quater note (resolution)
#define SEQUENCER_UPDATE_HZ 127  	// sequencer update rate
#define DEFAULT_BPM 120	   		// 60,000,000/DEFAULT_MPQN quaternotes per minute


#endif

