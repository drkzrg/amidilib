#!/bin/bash   
scons cross=y target=atarist cross=y debug=3 --tree=prune
m68k-atari-mint-stack -v ../bin/*.tos --size=128k
cp -v ../bin/*.tos /home/saulot/Pulpit/HD/0AMIDILIB
cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
