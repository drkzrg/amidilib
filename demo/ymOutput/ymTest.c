
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "c_vars.h"
#include "input/ikbd.h"
#include "ym2149/ym2149.h"

#include <osbind.h>

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ ym2149 sound output test..\n");
  printf("'[' or ']' - change octave -/+ \n");
  printf("[q-h] - play note\n");
  printf("1,2,3 - enable/disable tone for channel A/B/C \n");
  printf("7,8,9 - enable/disable noise for channel A/B/C \n");
  printf("',' or '.' - change envelope -/+ \n");
  printf("';' or ''' - change noise generator period -/+ \n");
  printf("'z' or 'x' - change channels amplitude -/+ \n");
  printf("[i] - show this help screen \n");
  printf("[spacebar] - turn off all sounds \n");
  printf("[Esc] - quit\n");
  printf("(c)Nokturnal 2010\n");
  printf("================================================\n");
  
}


int main(void) {
  
  turnOffKeyclick();
  
  printHelpScreen();

  ymChannelData ymRegisters[3];
  
  U8 noteBaseArray[]={24,36,48,60,72,84,96,108};
  
  BOOL bQuitFlag=FALSE;
  U16 input=0L;
  U8 idx=0;
  
  U8 currentBaseIdx=0;

  U8 currentEnvIdx=0;
  U8 noisegenPeriod=15;
  U8 channelAmp=15; 
  ymRegisters[CH_A].amp=channelAmp;
  ymRegisters[CH_B].amp=channelAmp;
  ymRegisters[CH_C].amp=channelAmp;
  
  ymRegisters[CH_A].noiseEnable=0;
  ymRegisters[CH_B].noiseEnable=0;
  ymRegisters[CH_C].noiseEnable=0;
  
  ymRegisters[CH_A].toneEnable=1;
  ymRegisters[CH_B].toneEnable=1;
  ymRegisters[CH_C].toneEnable=1;
  
  U8 noteBase=noteBaseArray[currentBaseIdx];
  U8 currentEnvelope=getEnvelopeId(currentEnvIdx);
  
  while(bQuitFlag!=TRUE){
    
      input=(U16)Crawcin();

      if(input!=0){
    
      switch(toupper((U8)input)){
    
	case 0x1b:{
	  bQuitFlag=TRUE;
	}
	break;
	case ' ':{
	  ymSoundOff();
	}
	break;
	
	case '1':{
	  if(ymRegisters[CH_A].toneEnable){
	    ymRegisters[CH_A].toneEnable=0;
	    printf("[Channel A] tone OFF\n");
	  }else{
	    ymRegisters[CH_A].toneEnable=1;
	    printf("[Channel A] tone ON\n");
	  }
	
	 setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  
	}break;
	
	case '2':{
	if(ymRegisters[CH_B].toneEnable){
	    ymRegisters[CH_B].toneEnable=0;
	    printf("[Channel B] tone OFF\n");
	  }else{
	    ymRegisters[CH_B].toneEnable=1;
	    printf("[Channel B] tone ON\n");
	  }
	  
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 
	}break;
	case '3':{
	if(ymRegisters[CH_C].toneEnable){
	    ymRegisters[CH_C].toneEnable=0;
	    printf("[Channel C] tone OFF\n");
	  }else{
	    ymRegisters[CH_C].toneEnable=1;
	    printf("[Channel C] tone ON\n");
	  }
	 setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  
	}break;
	
	case '7':{
	  if(ymRegisters[CH_A].noiseEnable){
	    ymRegisters[CH_A].noiseEnable=0;
	    printf("[Channel A] noise OFF\n");
	  }else{
	    ymRegisters[CH_A].noiseEnable=1;
	    printf("[Channel A] noise ON\n");
	  }
	  
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 
	}break;
	
	case '8':{
	  if(ymRegisters[CH_B].noiseEnable){
	    ymRegisters[CH_B].noiseEnable=0;
	    printf("[Channel B] noise OFF\n");
	  }else{
	    ymRegisters[CH_B].noiseEnable=1;
	    printf("[Channel B] noise ON\n");
	  }
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 
	}break;
	
	case '9':{
	if(ymRegisters[CH_C].noiseEnable){
	    ymRegisters[CH_C].noiseEnable=0;
	    printf("[Channel C] noise OFF\n");
	  }else{
	    ymRegisters[CH_C].noiseEnable=1;
	    printf("[Channel C] noise ON\n");
	  }
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 
	}break;
	
	case '[':{
	  if(currentBaseIdx!=0){
	    currentBaseIdx--;
	    printf("Changed octave to: %d\n",currentBaseIdx+1);
	    idx=idx-12;
	 
	    setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  }
	}break;
	case ']':{
	if(currentBaseIdx!=7){
	    currentBaseIdx++;
	    printf("Changed octave to: %d\n",currentBaseIdx+1);
	    idx=idx+12;
	    
	    setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  }
	}break;
	case ',':{
	  if(currentEnvIdx!=0){
	    currentEnvIdx--;
	    printf("Changed envelope to: %d\n",currentEnvIdx);
	    
	    setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  }
	
	}break;
	case '.':{
	if(currentEnvIdx!=7){
	   currentEnvIdx++;
	   printf("Changed envelope to: %d\n",currentEnvIdx);
	   setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  }
	}break;
	case ';':{
	 if(noisegenPeriod!=0){
	  noisegenPeriod--;
	   printf("Changed noise generator period to: %d\n",noisegenPeriod);
	    setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 }
	}break;
	case '\'':{
	if(noisegenPeriod!=31){
	  noisegenPeriod++;
	   printf("Changed noise generator period to: %d\n",noisegenPeriod);
	   setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 }
	}break;
	case 'Z':{
	  
	  if(channelAmp!=0){
	    channelAmp--;
	    
	    printf("Changed channel amplitude to: %d\n",channelAmp);
	    ymRegisters[CH_A].amp=channelAmp;
	    ymRegisters[CH_B].amp=channelAmp;
	    ymRegisters[CH_C].amp=channelAmp;
	    
	    setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	 
	  }
	}break;
	
	case 'X':{
	  if(channelAmp!=16){
	    channelAmp++;
	     printf("Changed channel amplitude to: %d\n",channelAmp);
	     ymRegisters[CH_A].amp=channelAmp;
	     ymRegisters[CH_B].amp=channelAmp;
	     ymRegisters[CH_C].amp=channelAmp;
  
	     setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	  }

	}break;
	case 'I':{
	  printHelpScreen();
	}break;
	case 'Q':{
	  
	  idx=noteBaseArray[currentBaseIdx]+0;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'A':{
	  idx=noteBaseArray[currentBaseIdx]+1;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'W':{
	  idx=noteBaseArray[currentBaseIdx]+2;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'S':{
	  idx=noteBaseArray[currentBaseIdx]+3;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'E':{
	  idx=noteBaseArray[currentBaseIdx]+4;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'D':{
	  idx=noteBaseArray[currentBaseIdx]+5;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;  
	
	case 'R':{
	  idx=noteBaseArray[currentBaseIdx]+6;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'F':{
	  idx=noteBaseArray[currentBaseIdx]+7;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'T':{
	  idx=noteBaseArray[currentBaseIdx]+8;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;  
	
	case 'G':{
	  idx=noteBaseArray[currentBaseIdx]+9;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'Y':{
	  idx=noteBaseArray[currentBaseIdx]+10;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
	case 'H':{
	  idx=noteBaseArray[currentBaseIdx]+11;
	  setYm2149(ymRegisters,idx,currentEnvIdx, noisegenPeriod);
	}
	break;
	
    }
      
    }
  
  }
  
  //turn off sound output 
  ymSoundOff();
  printf("Program terminated...\n");

return 0;
}

