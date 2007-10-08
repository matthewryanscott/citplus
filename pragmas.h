// --------------------------------------------------------------------------
// Citadel: Pragmas.H
//
// all the pragmas you want set


// turn all warnings on with borland c++ 3.0 and 4.0
#ifdef __BORLANDC__
    #pragma option -w+
    #pragma warn +bbf
    #pragma warn +eas
    #pragma warn +pin
    #pragma warn +amb
    #pragma warn +amp
    #pragma warn +asm
    #pragma warn +def
    #pragma warn +nod
    #pragma warn +pro
    #pragma warn +stv
    #pragma warn +use
    #pragma warn +cln
    #pragma warn +sig
    #pragma warn +bei
//  #pragma warn +dcl
    #pragma warn +cpt
    #pragma warn +dup
    #pragma warn +sus
    #pragma warn +rpt
//  #pragma warn +vrt
    #pragma warn +big
    #pragma warn +ucp
    #pragma warn +stu
    #pragma warn +ext
//  #pragma warn +fdt
#endif

// Turn on optimizations
#if __BORLANDC__ == 1120
    #pragma option -Os
    #pragma option -Ot
    #pragma option -G
    #pragma option -Og
    #pragma option -Oa
    #pragma option -O
    #pragma option -Ol
    #pragma option -Z
    #pragma option -Oe
    #pragma option -Ob
#ifndef WINCIT
    #pragma intrinsic memchr
    #pragma intrinsic memcmp
    #pragma intrinsic strchr
    #pragma intrinsic strlen
    #pragma intrinsic strncat
    #pragma intrinsic strcat
    #pragma intrinsic strncmp
    #pragma intrinsic strncpy
    #pragma intrinsic strrchr
#endif
    #pragma -Om
    #pragma -Op
    #pragma -Ov
#endif
