
<a href="images/amidilib_logo_no_bg.png">
  <img alt="AMIDILIB logo"
       src="images/amidilib_logo_no_bg.png"/>
</a>

[![Coverity Scan](https://scan.coverity.com/projects/13521/badge.svg)](https://scan.coverity.com/projects/n0kturnal-amidilib)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.md)

Introduction
------------

<a href="images/amidilib-mt32.png">
<img alt="AMIDILIB logo"
src="images/amidilib-mt32.png"/>
</a>

AMIDILIB is a C language library which allows easy replay of MIDI files in various formats via external midi port found in all 16/32 bit [Atari home computers](https://en.wikipedia.org/wiki/Atari_ST). 
It supports General Midi instruments (GM1 / GM2) and Roland MT-32 family of synths. Main motivation behind this library was providing midi music replay in games, game ports or other applications.
 
Library can also preprocess various midi data and export them to custom binary format (nkt), which is more suitable for use in programs and games - 
data doesn't have to be preprocessed which decreases significantly loading times, it's smaller and it's easier to integrate with existing projects.

If you like this project or others feel free to donate to help cover partially my development time and expenses related to hardware. Thank you!
 
<a href="https://www.paypal.com/donate/?hosted_button_id=UTEF2ABTDNSMC"><img alt="support amidilib" src="https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif"/></a>

Compilation from sources
------------------------

To compile library from sources following tools are needed:

* [SCons build tool](http://scons.org/)
* GCC C cross compiler supporting c99 - [Thorsten Otto's FreeMint cross tools page](https://tho-otto.de/crossmint.php) or [gcc(brownelf)](https://bitbucket.org/ggnkua/bigbrownbuild-git/src/master)  
* [VASM m68k cross compiler](http://sun.hasenbraten.de/vasm/)
* Environment like [Cygwin](https://www.cygwin.com/) under Windows or Linux.

Detailed information about building library are in [compilation instructions](compilation.md), basic integration steps with your own programs are in [integration](integration.md).

Currently there's no binary relases, but it might change in the future. Documentation and examples are also missing, but they will be provided at some point. 
Until then you can contact me: [nokturnal@nokturnal.pl](mailto:nokturnal@nokturnal.pl).

License
-------------------------
See [LICENSE](LICENSE.md)

Projects using AMIDILIB
-----------------------
[BadMood](http://www.leonik.net/dml/sec_bm.py) BadMooD is an enhanced version of (Linux) Ultimate Doom for Atari Falcon030. It was written specifically for a Atari Falcon030 @ 16MHz + DSP56k @ 32MHz
