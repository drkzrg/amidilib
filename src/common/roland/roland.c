
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "roland.h"
#include "midi_send.h"
#include "midi_cmd.h"

/* file contains all instrument tables for Roland MT-32 family devices */

U8 const arReset[8]={0xf0,0x41,0x10,0x16,0x12,0x7f,0x01,0xf7};

/**
* instruments can be assigned to 1-8 part (channel 2-9, can be remmaped to channel 1-8)
*/

const U8 *arCM32Linstruments[]={
"AcouPiano 1",
"AcouPiano 2",
"AcouPiano 3",
"ElecPiano 1",
"ElecPiano 2",
"ElecPiano 3",
"ElecPiano 4",
"Honkytonk",
"Elec Org 1",
"Elec Org 2",
"Elec Org 3",
"Elec Org 4",
"Pipe Org 1",
"Pipe Org 2",
"Pipe Org 3",
"Accordion",
"Harpsi 1",
"Harpsi 2",
"Harpsi 3",
"Clavi 1",
"Clavi 2",
"Clavi 3",
"Celesta 1",
"Celesta 2",
"SynBrass 1",
"SynBrass 2",
"SynBrass 3",
"SynBrass 4",
"Syn Bass 1",
"Syn Bass 2",
"Syn Bass 3",
"Syn Bass 4",
"Fantasy",
"Harmo Pan",
"Chorale",
"Glasses",
"Sountrack",
"Atmosphere",
"Warm Bell",
"Funny Vox",
"Echo Bell",
"Ice Rain",
"Oboe 2001",
"Echo Pan",
"DoctorSolo",
"Schooldaze",
"Bellsinger",
"SquareWave",
"Str Sect 1",
"Str Sect 2",
"Str Sect 3",
"Pizzicato",
"Violin 1",
"Violin 2",
"Cello 1",
"Cello 2",
"Contrabass",
"Harp 1",
"Harp 2",
"Guitar 1",
"Guitar 2",
"Elec Gtr 1",
"Elec Gtr 2",
"Sitar",
"AcouBass 1",
"AcouBass 2",
"ElecBass 1",
"ElecBass 2",
"SlapBass 1",
"SlapBass 2",
"Fretless 1",
"Fretless 2",
"Flute 1",
"Flute 2",
"Piccolo 1",
"Piccolo 2",
"Recorder",
"Pan Pipes",
"Sax 1",
"Sax 2",
"Sax 3",
"Sax 4",
"Clarinet 1",
"Clarinet 2",
"Oboe",
"Engl Horn",
"Bassoon",
"Harmonica",
"Trumpet 1",
"Trumpet 2",
"Trombone 1",
"Trombone 2",
"Fr Horn 1",
"Fr Horn 2",
"Tuba",
"Brs Sect 1",
"Brs Sect 2",
"Vibe 1",
"Vibe 2",
"Syn Mallet",
"Windbell",
"Glock",
"Tube Bell",
"Xylophone",
"Marimba",
"Koto",
"Sho",
"Shakuhachi",
"Whistle 1",
"Whistle 2",
"Bottleblow",
"BreathPipe",
"Timpani",
"MelodicTom",
"Deep Snare",
"ElecPerc 1",
"ElecPerc 2",
"Taiko",
"Taiko Rim",
"Cymbal",
"Castanets",
"Triangle",
"Orche Hit",
"Telephone",
"Bird Tweet",
"OneNoteJam",
"WaterBells",
"JungleTune"
};

/** 
* drumset like in MT-32, but with additional sfx *only* for part 10 (channel 11)
* the mapping is key note -> sound, program number 128 
*/

const U8 *arCM32Lrhythm[]={
"00 [no sound]",
"01 [no sound]",
"02 [no sound]",
"03 [no sound]",
"04 [no sound]",
"05 [no sound]",
"06 [no sound]",
"07 [no sound]",
"08 [no sound]",
"09 [no sound]",
"0A [no sound]",
"0B [no sound]",
"0C [no sound]",
"0D [no sound]",
"0E [no sound]",
"0F [no sound]",
"10 [no sound]",
"11 [no sound]",
"12 [no sound]",
"13 [no sound]",
"14 [no sound]",
"15 [no sound]",
"16 [no sound]",
"17 [no sound]",
"18 [no sound]",
"19 [no sound]",
"1A [no sound]",
"1B [no sound]",
"1C [no sound]",
"1D [no sound]",
"1E [no sound]",
"1F [no sound]",
"20 [no sound]",
"21 [no sound]",
"22 [no sound]",
"Acoustic Bass Drum",
"Acoustic Bass Drum",
"Rim Shot",
"Acoustic Snare Drum",
"Hand Clap",
"Electronic Snare Drum",
"Acoustic Low Tom",
"Closed High Hat",
"Acoustic Low Tom",
"Open High Hat 2",
"Acoustic Middle Tom",
"Open High Hat 1",
"Acoustic Middle Tom",
"Acoustic High Tom",
"Crash Cymbal",
"Acoustic High Tom",
"Ride Cymbal",
"34 [no sound]",
"35 [no sound]",
"Tambourine",
"37 [no sound]",
"Cowbell",
"39 [no sound]",
"3A [no sound]",
"3B [no sound]",
"High Bongo",
"Low Bongo",
"Mute High Conga",
"High Conga",
"Low Conga",
"High Timbale",
"Low Timbale",
"High Agogo",
"Low Agogo",
"Cabasa",
"Maracas",
"Short Whistle",
"Long Whistle",
"Quijada",
"4A [no sound]",
"Claves",
"Laughing",
"Screaming",
"Punch",
"Heartbeat",
"Footsteps1",
"Footsteps2",
"Applause",
"Creaking",
"Door",
"Scratch",
"Windchime",
"Engine",
"Car-stop",
"Car-pass",
"Crash",
"Siren",
"Train",
"Jet",
"Helicopter",
"Starship",
"Pistol",
"Machinegun",
"Lasergun",
"Explosion",
"Dog",
"Horse",
"Birds",
"Rain",
"Thunder",
"Wind",
"Waves",
"Stream",
"Bubble",
"6D [no sound]",
"6E [no sound]",
"6F [no sound]",
"70 [no sound]",
"71 [no sound]",
"72 [no sound]",
"73 [no sound]",
"74 [no sound]",
"75 [no sound]",
"76 [no sound]",
"77 [no sound]",
"78 [no sound]",
"79 [no sound]",
"7A [no sound]",
"7B [no sound]",
"7C [no sound]",
"7D [no sound]",
"7E [no sound]",
"7F [no sound]"
};


// program number #57
const U8 *arCM500sfx[]={
"0 [no sound]",
"1 [no sound]",
"2 [no sound]",
"3 [no sound]",
"4 [no sound]",
"5 [no sound]",
"6 [no sound]",
"7 [no sound]",
"8 [no sound]",
"9 [no sound]",
"10 [no sound]",
"11 [no sound]",
"12 [no sound]",
"13 [no sound]",
"14 [no sound]",
"15 [no sound]",
"16 [no sound]",
"17 [no sound]",
"18 [no sound]",
"19 [no sound]",
"20 [no sound]",
"21 [no sound]",
"22 [no sound]",
"23 [no sound]",
"24 [no sound]",
"25 [no sound]",
"26 [no sound]",
"27 [no sound]",
"28 [no sound]",
"29 [no sound]",
"30 [no sound]",
"31 [no sound]",
"32 [no sound]",
"33 [no sound]",
"34 [no sound]",
"35 [no sound]",
"36 [no sound]",
"37 [no sound]",
"38 [no sound]",
"High Q",
"Slap",
"Scratch Push",
"Scrach Pull",
"Sticks",
"Square Click",
"Metronome Click",
"Metronome Bell",
"Guitar sliding finger",
"Guitar cutting noise (down)",
"Guitar cutting noise (up)",
"String slap of double bass",
"Fl. Key Click",
"Laughing",
"Screaming",
"Punch",
"Heart Beat",
"Footsteps 1",
"Footsteps 2",
"Applause",
"Door Creaking",
"Door",
"Scratch",
"Windchime",
"Car-Engine",
"Car-Stop",
"Car-Pass",
"Car-Crash",
"Siren",
"Train",
"Jetplane",
"Helicopter",
"Starship",
"Gun Shot",
"Machine Gun",
"Lasergun",
"Explosion",
"Dog",
"Horse-Gallop",
"Birds",
"Rain",
"Thunder",
"Wind",
"Seashore",
"Stream",
"Bubble",
"85 [no sound]",
"86 [no sound]",
"87 [no sound]",
"88 [no sound]",
"89 [no sound]",
"90 [no sound]",
"91 [no sound]",
"92 [no sound]",
"93 [no sound]",
"94 [no sound]",
"95 [no sound]",
"96 [no sound]",
"97 [no sound]",
"98 [no sound]",
"99 [no sound]",
"100 [no sound]",
"101 [no sound]",
"102 [no sound]",
"103 [no sound]",
"104 [no sound]",
"105 [no sound]",
"106 [no sound]",
"107 [no sound]",
"108 [no sound]",
"109 [no sound]",
"110 [no sound]",
"111 [no sound]",
"112 [no sound]",
"113 [no sound]",
"114 [no sound]",
"115 [no sound]",
"116 [no sound]",
"117 [no sound]",
"118 [no sound]",
"119 [no sound]",
"120 [no sound]",
"121 [no sound]",
"122 [no sound]",
"123 [no sound]",
"124 [no sound]",
"125 [no sound]",
"126 [no sound]",
"127 [no sound]",
"128 [no sound]"
};

const U8 *arMT32instruments[]={
"AcouPiano 1",
"AcouPiano 2",
"AcouPiano 3",
"ElecPiano 1",
"ElecPiano 2",
"ElecPiano 3",
"ElecPiano 4",
"Honkytonk",
"Elec Org 1",
"Elec Org 2",
"Elec Org 3",
"Elec Org 4",
"Pipe Org 1",
"Pipe Org 2",
"Pipe Org 3",
"Accordion",
"Harpsi 1",
"Harpsi 2",
"Harpsi 3",
"Clavi 1",
"Clavi 2",
"Clavi 3",
"Celesta 1",
"Celesta 2",
"SynBrass 1",
"SynBrass 2",
"SynBrass 3",
"SynBrass 4",
"Syn Bass 1",
"Syn Bass 2",
"Syn Bass 3",
"Syn Bass 4",
"Fantasy",
"Harmo Pan",
"Chorale",
"Glasses",
"Sountrack",
"Atmosphere",
"Warm Bell",
"Funny Vox",
"Echo Bell",
"Ice Rain",
"Oboe 2001",
"Echo Pan",
"DoctorSolo",
"Schooldaze",
"Bellsinger",
"SquareWave",
"Str Sect 1",
"Str Sect 2",
"Str Sect 3",
"Pizzicato",
"Violin 1",
"Violin 2",
"Cello 1",
"Cello 2",
"Contrabass",
"Harp 1",
"Harp 2",
"Guitar 1",
"Guitar 2",
"Elec Gtr 1",
"Elec Gtr 2",
"Sitar",
"AcouBass 1",
"AcouBass 2",
"ElecBass 1",
"ElecBass 2",
"SlapBass 1",
"SlapBass 2",
"Fretless 1",
"Fretless 2",
"Flute 1",
"Flute 2",
"Piccolo 1",
"Piccolo 2",
"Recorder",
"Pan Pipes",
"Sax 1",
"Sax 2",
"Sax 3",
"Sax 4",
"Clarinet 1",
"Clarinet 2",
"Oboe",
"Engl Horn",
"Bassoon",
"Harmonica",
"Trumpet 1",
"Trumpet 2",
"Trombone 1",
"Trombone 2",
"Fr Horn 1",
"Fr Horn 2",
"Tuba",
"Brs Sect 1",
"Brs Sect 2",
"Vibe 1",
"Vibe 2",
"Syn Mallet",
"Windbell",
"Glock",
"Tube Bell",
"Xylophone",
"Marimba",
"Koto",
"Sho",
"Shakuhachi",
"Whistle 1",
"Whistle 2",
"Bottleblow",
"BreathPipe",
"Timpani",
"MelodicTom",
"Deep Snare",
"ElecPerc 1",
"ElecPerc 2",
"Taiko",
"Taiko Rim",
"Cymbal",
"Castanets",
"Triangle",
"Orche Hit",
"Telephone",
"Bird Tweet",
"OneNoteJam",
"WaterBells",
"JungleTune"
};

const U8 *arMT32rhythm[]={
"00 [no sound]",
"01 [no sound]",
"02 [no sound]",
"03 [no sound]",
"04 [no sound]",
"05 [no sound]",
"06 [no sound]",
"07 [no sound]",
"08 [no sound]",
"09 [no sound]",
"0A [no sound]",
"0B [no sound]",
"0C [no sound]",
"0D [no sound]",
"0E [no sound]",
"0F [no sound]",
"10 [no sound]",
"11 [no sound]",
"12 [no sound]",
"13 [no sound]",
"14 [no sound]",
"15 [no sound]",
"16 [no sound]",
"17 [no sound]",
"18 [no sound]",
"19 [no sound]",
"1A [no sound]",
"1B [no sound]",
"1C [no sound]",
"1D [no sound]",
"1E [no sound]",
"1F [no sound]",
"20 [no sound]",
"21 [no sound]",
"Acou BD",
"Acou BD",
"Rim Shot",
"Acou SD",
"Hand Clap",
"Elec SD",
"Acou Low Tom",
"Clsd Hi Hat",
"Acou Low Tom",
"Open Hi Hat 2",
"Acou Mid Tom",
"Open Hi Hat 1",
"Acou Mid Tom",
"Acou Mid Tom",
"Acou Hi Tom",
"Crash Cym",
"Acou Hi Tom",
"Ride Cym",
"34 [no sound]",
"35 [no sound]",
"Tambourine",
"37 [no sound]",
"Cowbell",
"39 [no sound]",
"3A [no sound]",
"3B [no sound]",
"High Bongo",
"Low Bongo",
"Mt High Conga",
"High Conga",
"Low Conga",
"High Timbale",
"Low Timbale",
"High Agogo",
"Low Agogo",
"Cabasa",
"Maracas",
"Smba Whis S",
"Smba Whis L",
"Quijada",
"4A [no sound]",
"Claves",
"4C [no sound]",
"4D [no sound]",
"4E [no sound]",
"4F [no sound]",
"50 [no sound]",
"51 [no sound]",
"52 [no sound]",
"53 [no sound]",
"54 [no sound]",
"55 [no sound]",
"56 [no sound]",
"57 [no sound]",
"58 [no sound]",
"59 [no sound]",
"5A [no sound]",
"5B [no sound]",
"5C [no sound]",
"5D [no sound]",
"5E [no sound]",
"5F [no sound]",
"60 [no sound]",
"61 [no sound]",
"62 [no sound]",
"63 [no sound]",
"64 [no sound]",
"65 [no sound]",
"66 [no sound]",
"67 [no sound]",
"68 [no sound]",
"69 [no sound]",
"6A [no sound]",
"6B [no sound]",
"6C [no sound]",
"6D [no sound]",
"6E [no sound]",
"6F [no sound]",
"70 [no sound]",
"71 [no sound]",
"72 [no sound]",
"73 [no sound]",
"74 [no sound]",
"75 [no sound]",
"76 [no sound]",
"77 [no sound]",
"78 [no sound]",
"79 [no sound]",
"7A [no sound]",
"7B [no sound]",
"7C [no sound]",
"7D [no sound]",
"7E [no sound]",
"7F [no sound]"
};



/* module default settings table */
// CM-32L specific
const U8 *getCM32LInstrName(U8 ubInstrNb){
 return(arCM32Linstruments[ubInstrNb]);
}

const U8 *getCM32LRhythmName(U8 ubNoteNb){
 return(arCM32Lrhythm[ubNoteNb]);
}

// cm-500 specific
const U8 *getCM500SFXName(U8 ubNoteNb){
 return(arCM500sfx[ubNoteNb]);
}

void  allPartsOnCm500(void){
 /* TODO: */
}

void  allPartsOffCm500(void)
{
 /* TODO: */
}

// all
U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end){
U8 total = 0 ;
U8 mask  = 0x7F ;

while ( buf_start <= buf_end ){
  total += *buf_start ;
  buf_start++ ;
}

 return (0x80 - (total & mask)) & mask ;
}

void MT32Reset(void){
#ifdef IKBD_MIDI_SEND_DIRECT
  for(int i=0;i<7;i++){
    MIDIsendBuffer[MIDIbytesToSend++]=arReset[i];
  }
  amMidiSendIKBD();
#else
    MIDI_SEND_DATA(8,arReset);
#endif
}


