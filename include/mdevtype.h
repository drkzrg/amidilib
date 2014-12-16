#ifndef MDEVTYPE_H
#define MDEVTYPE_H

/* some messages are ignored by specified sound source so why bother to process them? */
/* second thing: sometimes we need different treatment for GS and LA sound sources */
typedef enum _midiDeviceTypes{
  DT_LA_SOUND_SOURCE=0,     /* native MT32 */
  DT_LA_SOUND_SOURCE_EXT,   /* for extended MT 32 modules with extra patches like CM-32L/CM-64 */
  DT_GS_SOUND_SOURCE,       /* for pure GS sound source */
  DT_LA_GS_MIXED,           /* LA/GS mixed */
  DT_GM_SOUND_SOURCE,       /* for GM sound source */
  DT_MT32_GM_EMULATION,     /* before loading midi data MT32 sound banks has to be patched */
  DT_XG_GM_YAMAHA,
  DT_NUM_DEVICES	    /* before loading midi data MT32 sound banks has to be patched */
} eMidiDeviceType;


#endif // MDEVTYPE_H
