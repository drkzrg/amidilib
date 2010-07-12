
#include "ym2149.h"

void hMidiEvent(){
//dummy 
}

void ymDoSound(U8 hByte,U8 lByte){
    Giaccess(254,MIXER+128);		// sound chip setup
    Giaccess(15,AMP_OSC1+128);
    Giaccess(lByte,LB_OSC1+128);	// set note
    Giaccess(hByte,HB_OSC1+128);
}

void ymSoundOff(){
  Giaccess(255,MIXER+128);
}


