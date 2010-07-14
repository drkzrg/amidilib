
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <ctype.h> 
#include <osbind.h>

#include "c_vars.h"
 

extern void turnOffKeyclick(void);

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ midi output test..\n");
  printf("'[' or ']' - change octave -/+ \n");
  printf("[q-h] - play note\n");
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
  
  U8 noteBaseArray[]={24,36,48,60,72,84,96,108};
  
  BOOL bQuitFlag=FALSE;
  U16 input=0L;
  U8 idx=0;
  
  U8 currentBaseIdx=0;
  U8 currentEnvIdx=0;
  U8 noisegenPeriod=15;
  U8 channelAmp=15;
  
  U8 noteBase=noteBaseArray[currentBaseIdx];
  
  while(bQuitFlag!=TRUE){
    
    input=(U16)Crawcin();
    
    if(input!=0){
    
      switch(toupper((U8)input)){
    
	case 0x1b:{
	  bQuitFlag=TRUE;
	}
	break;
	case ' ':{
	 // turn off all sound
	}
	break;
	
	case '[':{
	  if(currentBaseIdx!=0){
	    currentBaseIdx--;
	    printf("Changed octave to: %d\n",currentBaseIdx+1);
	    idx=idx-12;
	    
	  }
	}break;
	case ']':{
	if(currentBaseIdx!=7){
	    currentBaseIdx++;
	    printf("Changed octave to: %d\n",currentBaseIdx+1);
	    idx=idx+12;
	    
	  }
	}break;
	
	
	case 'Z':{
	  if(channelAmp!=0){
	    channelAmp--;
	     printf("Changed channel amplitude to: %d\n",channelAmp);
	     
	  }
	}break;
	
	case 'X':{
	  if(channelAmp!=16){
	    channelAmp++;
	     printf("Changed channel amplitude to: %d\n",channelAmp);
	     
	  }

	}break;
	case 'I':{
	  printHelpScreen();
	}break;
	case 'Q':{
	  
	  idx=noteBaseArray[currentBaseIdx]+0;
	  
	}
	break;
	
	case 'A':{
	  
	  idx=noteBaseArray[currentBaseIdx]+1;
	  
	}
	break;
	
	case 'W':{
	  
	  idx=noteBaseArray[currentBaseIdx]+2;
	  
	}
	break;
	
	case 'S':{
	  
	  idx=noteBaseArray[currentBaseIdx]+3;
	  
	}
	break;
	
	case 'E':{
	  idx=noteBaseArray[currentBaseIdx]+4;
	  
	}
	break;
	
	case 'D':{
	  idx=noteBaseArray[currentBaseIdx]+5;
	  
	}
	break;  
	
	case 'R':{
	  idx=noteBaseArray[currentBaseIdx]+6;
	  
	}
	break;
	
	case 'F':{
	  idx=noteBaseArray[currentBaseIdx]+7;
	  
	}
	break;
	
	case 'T':{
	  idx=noteBaseArray[currentBaseIdx]+8;
	  
	}
	break;  
	
	case 'G':{
	  idx=noteBaseArray[currentBaseIdx]+9;
	  
	}
	break;
	
	case 'Y':{
	  idx=noteBaseArray[currentBaseIdx]+10;
	  
	}
	break;
	
	case 'H':{
	  idx=noteBaseArray[currentBaseIdx]+11;
	  
	}
	break;
	
    }
      
    }
  
  }
  
  //turn off sound output 
 
 printf("Program terminated...\n");

return 0;
}