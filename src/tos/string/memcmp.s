        
        XDEF    _memcmp
        
        TEXT      
_memcmp:
        move.l    4(sp),a0        ; string 1
        move.l    8(sp),a1        ; string 2
        move.l    12(sp),d0        ; length
        beq.s    .done        ; if zero, nothing to do
        move.l    a0,d1
        btst     #0,d1            ; string 1 address odd?
        beq.s    .even
        cmpm.b    (a0)+, (a1)+    ; yes, compare a byte
        bne.s    .noteq
        subq.l    #1,d0            ; adjust count
        bne.s    .done
.even:
        move.l    a1,d1
        btst    #0,d1            ; string 2 address odd?
        bne.s    .bloop            ; yes, no hope for alignment, compare bytes
        move.l    d0,d1            ; no, both even
        lsr.l    #2,d1            ; convert count to longword count
        beq.s    .bloop            ; count 0, skip longword loop

.lloop:
        cmpm.l (a0)+, (a1)+        ; compare a longword
        bne.s    .noteql            ; not equal, return non-zero
        subq.l    #1,d1            ; adjust count
        bne.s    .lloop            ; still more, keep comparing
        and.l    #3,d0            ; what remains
        beq.s    .done            ; nothing, all done
.bloop:
        cmpm.b     (a0)+,(a1)+        ; compare a byte
        bne.s    .noteq            ; not equal, return non-zero
        subq.l    #1,d0            ; adjust count
        bne.s    .bloop            ; still more, keep going
        rts

.noteql:
        subq.l    #4,a0
        subq.l    #4,a1
        move.l    #4,d0
        bra.s    .bloop
.noteq:
        clr.l    d0
        clr.l    d1
        move.b    -(a0),d0
        move.b    -(a1),d1
        sub.l    d1,d0
.done:
        rts