#!/bin/bash
# build helper script

#    Copyright 2007-2014 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

#general
#cygwin
tools_prefix='/opt/cross-mint'
base_dir='/home/saulot/'

#linux
#tools_prefix='/usr'
#base_dir='/home/saulot/'

debug_level=1
cross=1

##########################################################################################################################################

deb_str="";
cross_prefix="";
cross_str="";

case $debug_level in
    0)
        echo 'Release build'
        deb_str="debug=0"
        ;;
    1)
        echo 'Debug level 0'
        deb_str="debug=1"
        ;;
    2)
	
        echo 'Debug level 1'
        deb_str="debug=2"
        ;;
esac

if [ $cross -eq 1 ] ; then
    cross_prefix="m68k-atari-mint-"
    cross_str="cross=y"
else
    cross_str="cross=n"
fi

build_options="target=f030 prefix=$tools_prefix ikbd_direct=yes tx_enable=no conout=yes gemdos_io=yes $cross_str $deb_str"
copy_to_shared_dir=0
shared_dir='/home/saulot/Pulpit/shared/amidilib'

# hatari
# if copy_to_emu_dir=1 copy binaries to emulator directory
copy_to_emu_dir=0
run_emu=0
#install_dir=$base_dir'Pulpit/HD/AMIDIDEV/'
emu_parameters='--monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2'
#emu_dir=$base_dir'Pulpit/HD/'
#emu_dir='/cygdrive/d/Emulatory/TwardzielST/C/'
emu_dir='/cygdrive/d/Emulatory/HATARI/HD/'

install_dir=$emu_dir'AMIDIDEV/'

stack_size=64k

# output binaries
MIDIREP_BIN='midiplay.ttp'
YM2149_TEST_BIN='ym2149.tos'
MIDIOUT_BIN='midiout.tos'
MIDISEQ_BIN='midiseq.tos'
NKTREP_BIN='nktrep.ttp'
MID2NKT_BIN='mid2nkt.ttp'

# remote machine settings
# if send_to_native_machine=1 copy binaries to remote native machine via curl
send_to_native_machine=1
execute_on_remote=0
remote_exec=$MIDIOUT_BIN
remote_parm=''
REMOTE_MACHINE='192.168.0.3'
REMOTE_PATH='/c/amidilib/'

function process(){

if [ -f ../bin/$1 ];
then
   $tools_prefix$cross_prefix"stack" ../bin/$1 --size=$stack_size
   $tools_prefix$cross_prefix"flags" -S ../bin/$1

   if [ $debug_level -eq 0 ]
   then
        echo Stripping symbols from $1
        $tools_prefix$cross_prefix"strip" -s ../bin/$1
   fi

    if [ $send_to_native_machine -eq 1 ]
    then
        echo Sending $1 to $REMOTE_MACHINE
        curl -H "Expect:" --request POST --data-binary "@../bin/$1" $REMOTE_MACHINE$REMOTE_PATH$1
    fi

   if [ $copy_to_emu_dir -eq 1 ]
   then
        cp -v ../bin/$1 $install_dir
   fi
fi

}

function delete_if_exists(){
if [ -f ../bin/$1 ];
then
   rm ../bin/$1
fi
}

# delete binaries if they exist
echo "############################# Cleaning build .. "
delete_if_exists $MIDIREP_BIN
delete_if_exists $YM2149_TEST_BIN
delete_if_exists $MIDIOUT_BIN
delete_if_exists $MIDISEQ_BIN
delete_if_exists $NKTREP_BIN
delete_if_exists $MID2NKT_BIN

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

process $MIDIREP_BIN
process $YM2149_TEST_BIN
process $MIDIOUT_BIN
process $MIDISEQ_BIN
process $NKTREP_BIN
process $MID2NKT_BIN

#copy binaries to shared dir
if [ $copy_to_shared_dir -eq 1 ]
then
    cp -v ../bin/*.tos $shared_dir
    cp -v ../bin/*.ttp $shared_dir
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
