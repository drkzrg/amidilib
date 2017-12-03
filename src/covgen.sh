#!/bin/bash

scons --sconstruct=SConstruct_brownerelf -c
~/cov-analysis/bin/cov-build --dir cov-int sh -c "scons --sconstruct=SConstruct_brownerelf"
tar czvf amidilib.tgz cov-int

