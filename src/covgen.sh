#!/bin/bash
#tool config (once)
#~/cov-analysis/bin/cov-configure --comptype gcc --compiler /opt/cross-mint/bin/m68k-ataribrown-elf-gcc
#scons --sconstruct=SConstruct_brownelf -c
scons --sconstruct=SConstruct -c
#~/cov-analysis/bin/cov-build --dir cov-int bash -c "scons --sconstruct=SConstruct_brownelf"
~/cov-analysis/bin/cov-build --dir cov-int bash -c "scons --sconstruct=SConstruct"
tar czvf amidilib.tgz cov-int
