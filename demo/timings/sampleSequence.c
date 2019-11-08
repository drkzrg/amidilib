
#include "sampleSequence.h"

//sample sequence data
// output, test sequence for channel 1
  sEvent testSequenceChannel1[]={
  {0L,24,0xAD},
  {16L,25,0xAD},
  {32L,26,0xAD},
  {64L,27,0xAD},
  {96L,28,0xAD},
  {128L,29,0xAD},
  {16L,31,0xAD},
  {32L,32,0xAD},
  {64L,33,0xAD},
  {96L,34,0xAD},
  {128L,35,0xAD},
  {16L,37,0xAD},
  {32L,38,0xAD},
  {64L,39,0xAD},
  {96L,40,0xAD},
  {0L,0,0xAD}
};

// output test sequence for channel 2
 sEvent testSequenceChannel2[]={
  {0L,24,0xAD},
  {16L,25,0xAD},
  {32L,26,0xAD},
  {64L,27,0xAD},
  {96L,28,0xAD},
  {128L,29,0xAD},
  {16L,31,0xAD},
  {32L,32,0xAD},
  {64L,33,0xAD},
  {96L,34,0xAD},
  {128L,35,0xAD},
  {16L,37,0xAD},
  {32L,38,0xAD},
  {64L,39,0xAD},
  {96L,40,0xAD},
  {0L,0,0xAD}
};

// output test sequence for channel 2
 sEvent testSequenceChannel3[]={
  {0L,24,0xAD},
  {16L,25,0xAD},
  {32L,26,0xAD},
  {64L,27,0xAD},
  {96L,28,0xAD},
  {128L,29,0xAD},
  {16L,31,0xAD},
  {32L,32,0xAD},
  {64L,33,0xAD},
  {96L,34,0xAD},
  {128L,35,0xAD},
  {0L,36,0xAD},
  {16L,37,0xAD},
  {32L,38,0xAD},
  {64L,39,0xAD},
  {96L,40,0xAD},
  {0L,0,0xAD}
};

 sEvent *getTestSequenceChannel(uint8 chNb){
     switch(chNb){
        case 0: return &testSequenceChannel1[0]; break;
        case 1: return &testSequenceChannel2[0]; break;
        case 2: return &testSequenceChannel3[0]; break;
        default: return 0;break;
     };
 }
