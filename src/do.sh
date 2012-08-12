#!/bin/bash   
scons cross=y target=f030 debug=0 ikbd_direct=no --tree=prune
m68k-atari-mint-stack ../bin/*.tos --size=128k
m68k-atari-mint-stack ../bin/*.ttp --size=128k
m68k-atari-mint-flags -S ../bin/*.tos

m68k-atari-mint-flags -S ../bin/midiplay.ttp
m68k-atari-mint-flags -S ../bin/ym2149.tos
m68k-atari-mint-flags -S ../bin/midiseq.tos
m68k-atari-mint-flags -S ../bin/timings.tos

cp -v ../bin/*.tos /home/saulot/Pulpit/HD/0AMIDILIB
cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
cp -v ../bin/*.ttp /home/saulot/Pulpit/HD/0AMIDILIB
cp -v ../bin/*.ttp /home/saulot/Pulpit/shared/amidilib

