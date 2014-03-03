#!/bin/bash
# build helper script

#    Copyright 2007-2013 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

tools_prefix='/usr'

# hatari
install_dir='/home/saulot/Pulpit/HD/AMIDIDEV/'
emu_parameters='--monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2'
emu_dir='/home/saulot/Pulpit/HD/'
stack_size=128k

# remote machine settings
REMOTE_MACHINE='192.168.0.5'
REMOTE_PATH='/c/amidilib/'

MIDIREP_BIN='midiplay.ttp'
YM2149_TEST_BIN='ym2149.tos'
MIDIOUT_BIN='midiout.tos'
MIDISEQ_BIN='midiseq.tos'

# delete binaries if they exist
echo "############################# Cleaning build .. "
if [ -f ../bin/$MIDIREP_BIN ];
then 
   rm ../bin/$MIDIREP_BIN
fi
if [ -f ../bin/$YM2149_TEST_BIN ];
then 
   rm ../bin/YM2149_TEST_BIN
fi
if [ -f ../bin/$MIDIOUT_BIN ];
then 
   rm ../bin/$MIDIOUT_BIN
fi
if [ -f ../bin/$MIDISEQ_BIN ];
then 
   rm ../bin/$MIDISEQ_BIN
fi

build_options="cross=y target=f030 debug=1 prefix=$tools_prefix ikbd_direct=yes"

#clean all stuff
scons $build_options -c

#launch build
echo "############################# Starting build ... "   
scons $build_options

if [ "$tools_prefix" != "" ]; 
then 
   tools_prefix=$tools_prefix'/bin/'	
fi

#set proper flags
#copy binaries to install dir

echo "############################# Installation .. "
if [ -f ../bin/$MIDIREP_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" "../bin/"$MIDIREP_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$MIDIREP_BIN
   curl -H "Expect:" --request POST --data-binary "@../bin/$MIDIREP_BIN" $REMOTE_MACHINE$REMOTE_PATH$MIDIREP_BIN
   cp -v ../bin/$MIDIREP_BIN $install_dir
fi

if [ -f ../bin/$YM2149_TEST_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/$YM2149_TEST_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$YM2149_TEST_BIN
   curl -H "Expect:" --request POST --data-binary "@../bin/$YM2149_TEST_BIN" $REMOTE_MACHINE$REMOTE_PATH$YM2149_TEST_BIN
   cp -v ../bin/$YM2149_TEST_BIN $install_dir
fi
if [ -f ../bin/$MIDIOUT_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/$MIDIOUT_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$MIDIOUT_BIN
   curl -H "Expect:" --request POST --data-binary "@../bin/$MIDIOUT_BIN" $REMOTE_MACHINE$REMOTE_PATH$MIDIOUT_BIN
   cp -v ../bin/$MIDIOUT_BIN $install_dir
fi
if [ -f ../bin/$MIDISEQ_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/$MIDISEQ_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$MIDISEQ_BIN
   curl -H "Expect:" --request POST --data-binary "@../bin/$MIDISEQ_BIN" $REMOTE_MACHINE$REMOTE_PATH$MIDISEQ_BIN
   cp -v ../bin/$MIDISEQ_BIN $install_dir
fi

#curl -H "Expect:" -X RUN $REMOTE_MACHINE$REMOTE_PATH$(MIDI_REPLAY_BIN)


#copy binaries to shared dir
#cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
#cp -v ../bin/*.ttp /home/saulot/Pulpit/shared/amidilib

#uncomment following line to launch emulator after build
#hatari $emu_parameters -d $emu_dir 


