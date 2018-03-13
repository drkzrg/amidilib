#!/bin/bash
scons --sconstruct=SConstruct_brownelf -c
~/cov-analysis/bin/cov-build --dir cov-int sh -c "scons --sconstruct=SConstruct_brownelf"
#~/cov-analysis/bin/cov-build --dir cov-int bash -c "scons --sconstruct=SConstruct_brownerelf"

