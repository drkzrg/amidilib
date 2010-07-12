
#include "ym2149.h"

void ymDoSound(U8 hByte,U8 lByte, U8 envelope, U8 amp, U16 period,U8 noiseGenPeriod){
    
    Giaccess(0b11110000,MIXER+128);	// sound chip setup
    
    Giaccess(16,AMP_OSC1+128);		// set osc1
    Giaccess(lByte,LB_OSC1+128);	
    Giaccess(hByte,HB_OSC1+128);
    
    Giaccess(15,AMP_OSC2+128);		// set osc2
    Giaccess(lByte,LB_OSC2+128);	
    Giaccess(hByte,HB_OSC2+128);
    
    Giaccess(16,AMP_OSC3+128);		// set osc3
    Giaccess(lByte,LB_OSC3+128);	
    Giaccess(hByte,HB_OSC3+128);
    
    //set noise generator period 0-31
    if(noiseGenPeriod>31) noiseGenPeriod=31;
    Giaccess(noiseGenPeriod,NOISE_GEN+128);
    
    //set envelope period
    U8 lPeriod=(U8)(0x00FF&period);
    U8 hPeriod=(U8)(0x00FF&(period>>4));
    
    Giaccess(lPeriod,LB_ENV_PERIOD+128);
    Giaccess(hPeriod,HB_ENV_PERIOD+128);
    
    //set envelopes
    Giaccess(envelope,ENV_SELECT+128);
    
}

void ymSoundOff(){
  Giaccess(0b11111111,MIXER+128);
  
  //turn off envelope period
  Giaccess(0,LB_ENV_PERIOD+128);
  Giaccess(0,HB_ENV_PERIOD+128);
}


