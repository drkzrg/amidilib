#!/bin/bash

brownout -i ../bin/mid2nkt.elf -o ../bin/mid2nkt.ttp +x
brownout -i ../bin/midiout.elf -o ../bin/midiout.tos +x
brownout -i ../bin/midiplay.elf -o ../bin/midiplay.ttp +x
brownout -i ../bin/midiseq.elf -o ../bin/midiseq.tos +x
brownout -i ../bin/nktrep.elf -o ../bin/nktrep.ttp +x
brownout -i ../bin/ym2149.elf -o ../bin/ym2149.tos +x
