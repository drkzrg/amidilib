#!/bin/bash   
scons cross=y target=atarist cross=y debug=2 --tree=prune
m68k-atari-mint-stack ../bin/*.tos --size=128k
m68k-atari-mint-stack ../bin/*.ttp --size=128k
cp -v ../bin/*.tos /home/saulot/Pulpit/HD/0AMIDILIB
cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
cp -v ../bin/*.ttp /home/saulot/Pulpit/HD/0AMIDILIB
cp -v ../bin/*.ttp /home/saulot/Pulpit/shared/amidilib

