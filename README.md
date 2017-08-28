<a href="https://scan.coverity.com/projects/n0kturnal-amidilib">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/13521/badge.svg"/>
</a>
<a href="images/amidilib_logo_no_bg.png">
  <img alt="AMIDILIB logo"
       src="images/amidilib_logo_no_bg.png"/>
</a>

## Welcome to AMIDILIB project pages

### Introduction
AMIDILIB is replay library which allows easy replay of MIDI files in various formats via external midi port found in all 16/32 bit Atari's. It supports General Midi (GM1 / GM2) and Roland MT-32 family of synths. It can also preprocess various midi data and export it to custom binary format, which is more suitable to use in your own programs and games - data doesn't have to be preprocessed and contain minimal amount of information needed.   

### Compilation from sources

To compile library from sources several tools are needed:
* SCons build tool (http://scons.org/)
* gcc cross compiler - gcc 4.3.4 (http://vincent.riviere.free.fr/soft/m68k-atari-mint/) and gcc 7.10 (http://d-bug.mooo.com/beyondbrown/post/gcc7/)
* VASM m68k cross compiler (http://sun.hasenbraten.de/vasm/)
* Environment like Cygwin or Linux with bash.



