#!/bin/bash

scons --sconstruct=SConstruct_brownerelf -c
~/cov-analysis/bin/cov-build --dir cov-int scons --sconstruct=SConstruct_brownerelf
tar czvf amidilib.tgz cov-int

