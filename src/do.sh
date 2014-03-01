#!/bin/bash
# build helper script

#    Copyright 2007-2013 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

tools_prefix='/usr/'
install_dir='/home/saulot/Pulpit/HD/AMIDIDEV/'
emu_parameters='--monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2'
emu_dir='/home/saulot/Pulpit/HD/'
stack_size=128k

#delete binaries if they exist
echo "############################# Cleaning build .. "
if [ -f ../bin/midiplay.ttp ]; 
then 
   rm ../bin/midiplay.ttp
fi
if [ -f ../bin/ym2149.tos ]; 
then 
   rm ../bin/ym2149.tos
fi
if [ -f ../bin/midiseq.tos ]; 
then 
   rm ../bin/midiseq.tos
fi
if [ -f ../bin/timings.tos ]; 
then 
   rm ../bin/timings.tos
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
if [ -f ../bin/midiplay.ttp ]; 
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/midiplay.ttp --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/midiplay.ttp
   cp -v ../bin/midiplay.ttp $install_dir
fi

if [ -f ../bin/ym2149.tos ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/ym2149.tos --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/ym2149.tos
   cp -v ../bin/ym2149.tos $install_dir
fi
if [ -f ../bin/midiseq.tos ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/midiseq.tos --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/midiseq.tos
   cp -v ../bin/midiseq.tos $install_dir
fi
if [ -f ../bin/timings.tos ];
then 
   $tools_prefix"m68k-atari-mint-stack" ../bin/timings.tos --size=$stack_size
   $tools_prefix"m68k-atari-mint-flags" -S ../bin/timings.tos
   cp -v ../bin/timings.tos $install_dir
fi

#copy binaries to shared dir
#cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
#cp -v ../bin/*.ttp /home/saulot/Pulpit/shared/amidilib

#uncomment following line to launch emulator after build
#hatari $emu_parameters -d $emu_dir 


