#!/bin/bash   
scons cross=y target=f030 cross=y debug=2 --tree=prune
cp -v ../bin/*.tos /home/saulot/Pulpit/HD/0AMIDILIB
cp -v ../bin/*.tos /home/saulot/Pulpit/shared/amidilib
