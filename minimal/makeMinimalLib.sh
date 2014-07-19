#!/bin/bash
# build helper script

#    Copyright 2007-2014 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

#general
#cygwin
#tools_prefix='/opt/cross-mint'
#base_dir='/home/saulot/'

#linux
tools_prefix='/usr'
base_dir='/home/saulot/'

build_options="cross=y target=f030 debug=0 prefix=$tools_prefix ikbd_direct=yes"

#clean all stuff
scons $build_options -c

#launch build
echo "############################# Starting build ... "
scons $build_options
