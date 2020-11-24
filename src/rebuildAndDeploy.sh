#!/bin/bash
# deploy/rebuild helper script

#    Copyright 2007-2020 Pawel Goralski
#    
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

args=("$@") 
ARGS=${#args[@]} 
cleanall=0 
stripsymbols=0

for (( i=0;i<$ARGS;++i)); do 
if [ ${args[${i}]} = "--clean" ]; then
  cleanall=1
fi  
if [ ${args[${i}]} = "--strip" ]; then
  stripsymbols=1
fi
done

copy_to_shared_dir=0
shared_dir='/home/saulot/Pulpit/shared/amidilib'

# hatari / emulator
# if copy_to_emu_dir=1 copy binaries to emulator directory
copy_to_emu_dir=0
run_emu=0
#install_dir=$base_dir'Pulpit/HD/AMIDIDEV/'
emu_parameters='--monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2'
emu_dir=$base_dir'Pulpit/HD/'
#emu_dir='/cygdrive/d/Emulatory/TwardzielST/C/'
#emu_dir='/cygdrive/d/Emulatory/HATARI/HD/'
install_dir=$emu_dir'AMIDIDEV/'

stack_size=16k

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
REMOTE_MACHINE='6.6.6.15'
REMOTE_PATH='/e/adebug/'

tools_prefix='/opt/cross-mint/bin/'
cross_prefix='m68k-ataribrown-elf-'
BUILD_CONFIG='debug'
BUILD_DIR='../build/brownelf/'$BUILD_CONFIG'/'

function process()
{
echo Processing $BUILD_DIR$1
if [ -f $BUILD_DIR$1 ];
then
#   $tools_prefix$cross_prefix"stack" $BUILD_DIR$1 --size=$stack_size
#   $tools_prefix$cross_prefix"flags" -S $BUILD_DIR$1

   if [ $stripsymbols -eq 1 ]
   then
        echo Stripping symbols from $1
        $tools_prefix$cross_prefix"strip" -s $BUILD_DIR$1
   fi

    if [ $send_to_native_machine -eq 1 ]
    then
        echo Sending $1 to $REMOTE_MACHINE$REMOTE_PATH$1
        curl -0T "$BUILD_DIR$1" $REMOTE_MACHINE$REMOTE_PATH$1
    fi

   if [ $copy_to_emu_dir -eq 1 ]
   then
        cp -v $BUILD_DIR$1 $install_dir
   fi
fi
}

function delete_if_exists()
{
if [ -f $BUILD_DIR$1 ];     
then
   rm $BUILD_DIR$1
fi
}

# delete binaries if they exist
echo "############################# Removing binaries .. "
delete_if_exists $MIDIREP_BIN
delete_if_exists $YM2149_TEST_BIN
delete_if_exists $MIDIOUT_BIN
delete_if_exists $MIDISEQ_BIN
delete_if_exists $NKTREP_BIN
delete_if_exists $MID2NKT_BIN

# clean all stuff
if [ $cleanall -eq 1 ]; then
  echo "############################# Cleaning build .. "
  scons --sconstruct=SConstruct_brownelf_$BUILD_CONFIG -c
fi

#launch build
echo "############################# Starting build ... "   
scons --sconstruct=SConstruct_brownelf_$BUILD_CONFIG

process $MIDIREP_BIN
process $YM2149_TEST_BIN
process $MIDIOUT_BIN
process $MIDISEQ_BIN
process $NKTREP_BIN
process $MID2NKT_BIN

echo "############################## Done .."
