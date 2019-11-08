#!/bin/bash
# build helper script

#    Copyright 2007-2019 Pawel Goralski
#    
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.


copy_to_shared_dir=0
shared_dir='/home/saulot/Pulpit/shared/amidilib'

# hatari
# if copy_to_emu_dir=1 copy binaries to emulator directory
copy_to_emu_dir=1
run_emu=0
#install_dir=$base_dir'Pulpit/HD/AMIDIDEV/'
emu_parameters='--monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2'
emu_dir=$base_dir'Pulpit/HD/'
#emu_dir='/cygdrive/d/Emulatory/TwardzielST/C/'
#emu_dir='/cygdrive/d/Emulatory/HATARI/HD/'

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
send_to_native_machine=0
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
scons -c

#launch build
echo "############################# Starting build ... "   
scons 

