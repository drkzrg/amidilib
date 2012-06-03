#ifndef __MIDITIM_H__
#define __MIDITIM_H__

#include "c_vars.h"

#ifdef TIME_CHECK_PORTABLE
#include <time.h>
#endif


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
#ifdef TIME_CHECK_PORTABLE
/** utility for measuring function execution time (end-begin timestamp in ms) 
*   @param end - end timestamp
*   @param begin - begin timestamp
*/
double am_diffclock(clock_t end, clock_t begin);
#endif

/** get current time signature  
* @return returns current time signature in ms
*/

float getTimeStamp();

/** Utility function for measuring delta time  
 *   @return delta time since last getTimeStamp() function call in ms
 */
float getTimeDelta();

static const U32 DEFAULT_MPQN=500000;	   // tempo change step in ms
static const U32 DEFAULT_PPQN=120;	   // default pulses per quater note
static const U32 SEQUENCER_UPDATE_HZ=127;  // sequencer update rate
static const U32 DEFAULT_TEMPO=120;	   //60000000/500000 quaternotes per minute


#endif

