#!/bin/bash
# build helper script

#    Copyright 2007-2014 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

#general
tools_prefix='/usr'
base_dir='/home/saulot/'
copy_to_shared_dir=0

# hatari
# if copy_to_emu_dir=1 copy binaries to emulator directory
copy_to_emu_dir=1
run_emu=0
install_dir=$base_dir'Pulpit/HD/AMIDIDEV/'
emu_parameters='--monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2'
emu_dir=$base_dir'Pulpit/HD/'
stack_size=64k

# output binaries
MIDIREP_BIN='midiplay.ttp'
YM2149_TEST_BIN='ym2149.tos'
MIDIOUT_BIN='midiout.tos'
MIDISEQ_BIN='midiseq.tos'

# remote machine settings
# if send_to_native_machine=1 copy binaries to remote native machine via curl
send_to_native_machine=0
execute_on_remote=0
remote_exec=$MIDIOUT_BIN
remote_parm=''
REMOTE_MACHINE='192.168.0.5'
REMOTE_PATH='/c/amidilib/'

# delete binaries if they exist
echo "############################# Cleaning build .. "
if [ -f ../bin/$MIDIREP_BIN ];
then 
   rm ../bin/$MIDIREP_BIN
fi
if [ -f ../bin/$YM2149_TEST_BIN ];
then 
   rm ../bin/$YM2149_TEST_BIN
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
   if [ $send_to_native_machine -eq 1 ]
   then
       curl -H "Expect:" --request POST --data-binary "@../bin/$MIDIREP_BIN" $REMOTE_MACHINE$REMOTE_PATH$MIDIREP_BIN
   fi
   cp -v ../bin/$MIDIREP_BIN $install_dir
fi

if [ -f ../bin/$YM2149_TEST_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/$YM2149_TEST_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$YM2149_TEST_BIN

   if [ $send_to_native_machine -eq 1 ]
   then
   curl -H "Expect:" --request POST --data-binary "@../bin/$YM2149_TEST_BIN" $REMOTE_MACHINE$REMOTE_PATH$YM2149_TEST_BIN
   fi

   if [ $copy_to_emu_dir -eq 1 ]
   then
   cp -v ../bin/$YM2149_TEST_BIN $install_dir
   fi
fi
if [ -f ../bin/$MIDIOUT_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/$MIDIOUT_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$MIDIOUT_BIN

   if [ $send_to_native_machine -eq 1 ]
   then
        curl -H "Expect:" --request POST --data-binary "@../bin/$MIDIOUT_BIN" $REMOTE_MACHINE$REMOTE_PATH$MIDIOUT_BIN
   fi

   if [ $copy_to_emu_dir -eq 1 ]
   then
        cp -v ../bin/$MIDIOUT_BIN $install_dir
   fi

fi
if [ -f ../bin/$MIDISEQ_BIN ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/$MIDISEQ_BIN --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/$MIDISEQ_BIN

    if [ $send_to_native_machine -eq 1 ]
    then
        curl -H "Expect:" --request POST --data-binary "@../bin/$MIDISEQ_BIN" $REMOTE_MACHINE$REMOTE_PATH$MIDISEQ_BIN
    fi

   if [ $copy_to_emu_dir -eq 1 ]
   then
        cp -v ../bin/$MIDISEQ_BIN $install_dir
   fi
fi

#copy binaries to shared dir
if [ $copy_to_shared_dir -eq 1 ]
then
    cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
    cp -v ../bin/*.ttp /home/saulot/Pulpit/shared/amidilib
fi

#launch emulator after build
if [ $run_emu -eq 1 ]
then
    hatari $emu_parameters -d $emu_dir
fi

if [ $execute_on_remote -eq 1 ]
then
    echo "Launching: "$remote_exec
    curl -H "Expect:" -X RUN $REMOTE_MACHINE$REMOTE_PATH$remote_exec
fi
