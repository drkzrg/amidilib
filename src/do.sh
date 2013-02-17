#!/bin/bash

#clean all stuff
scons -c
rm ../bin/midiplay.ttp
rm ../bin/ym2149.tos
rm ../bin/midiseq.tos
rm ../bin/timings.tos

#build   
scons cross=y target=atarist debug=0 ikbd_direct=yes --tree=prune
m68k-atari-mint-stack ../bin/*.tos --size=128k
m68k-atari-mint-stack ../bin/*.ttp --size=128k
m68k-atari-mint-flags -S ../bin/*.tos

#set proper flags
m68k-atari-mint-flags -S ../bin/midiplay.ttp
m68k-atari-mint-flags -S ../bin/ym2149.tos
m68k-atari-mint-flags -S ../bin/midiseq.tos
m68k-atari-mint-flags -S ../bin/timings.tos

#adjust stack

cp -v ../bin/*.tos /home/saulot/Pulpit/HD/AMIDIDEV
#cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
cp -v ../bin/*.ttp /home/saulot/Pulpit/HD/AMIDIDEV
#cp -v ../bin/*.ttp /home/saulot/Pulpit/shared/amidilib
#hatari --monitor vga --memsize 14 --bpp 8 --drive-led y --confirm-quit no --midi-in /dev/midi2 --midi-out /dev/midi2 --conout 2  -d /home/saulot/Pulpit/HD/ 
