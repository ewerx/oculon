/*
    convenience.h
    Simple convenience macros
 
    syfluid - Mirko's interactive fluidum
    done by sy2002 in 2013
 
    contact me at code@sy2002.de
 
    Licensed under CC BY 3.0: http://creativecommons.org/licenses/by/3.0/
    You are free to Share (to copy, distribute and transmit the work),
    to Remix (to adapt the work), to make commercial use of the work
    as long as you credit it to sy2002 and link to http://www.sy2002.de
 
    Based on Jos Stam's "Real-Time Fluid Dynamics for Games"
    http://www.dgp.toronto.edu/people/stam/reality/Research/pdf/GDC03.pdf
*/

#pragma once

//MacOS 10.6 specifc issue
#ifndef GL_RGBA32F
    #define GL_RGBA32F GL_RGBA32F_ARB
#endif

#ifndef MIN
    #define MIN(__a, __b) ((__a) < (__b) ? (__a) : (__b))
#endif

#ifndef MAX
    #define MAX(__a, __b) ((__a) > (__b) ? (__a) : (__b))
#endif

#define RANGE(__x, __minx, __maxx) (MAX(MIN((__x), (__maxx)), (__minx)))
