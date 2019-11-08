#!/bin/bash
# build helper script

#    Copyright 2007-2019 Pawel Goralski
#    
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.
#    This file is part of BadMood/Amidilib.

#clean all stuff
echo ############ Clean All
scons --sconstruct=SConstruct_release -c

#build
echo ############ Build All
scons --sconstruct=SConstruct_release

#remove all intermediate files

echo ############ Cleanup
find ./nktlib/ -name "*.o" -type f|xargs rm -f

tar czvf nktlib.tgz ./nktlib


echo Done

