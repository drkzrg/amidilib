#!/bin/bash
# build helper script

#    Copyright 2007-2017 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.
#    This file is part of BadMood/Amidilib.

#clean all stuff
scons --sconstruct=Sconstruct_release -c

#build
scons --sconstruct=Sconstruct_release


