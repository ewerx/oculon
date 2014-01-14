/*
    frag_linsolve.glsl
 
    syfluid - Mirko's interactive fluidum
    done by sy2002 in 2011 and 2013
 
    contact me at code@sy2002.de
 
    Licensed under CC BY 3.0: http://creativecommons.org/licenses/by/3.0/
    You are free to Share (to copy, distribute and transmit the work),
    to Remix (to adapt the work), to make commercial use of the work
    as long as you credit it to sy2002 and link to http://www.sy2002.de
 
    Based on Jos Stam's "Real-Time Fluid Dynamics for Games"
    http://www.dgp.toronto.edu/people/stam/reality/Research/pdf/GDC03.pdf
*/

#version 110

uniform sampler2D x;
uniform sampler2D x0;
uniform float a;
uniform float c;
uniform float OneNStep;
uniform float OneMinusOneNStep;

void main()
{   
    //foreachelement in [1..N]
    //x[IX(i, j)] = (x0[IX(i, j)] +  a * (x[IX(i - 1, j)] + x[IX(i + 1, j)] + x[IX(i, j - 1)] + x[IX(i, j + 1)])) / c;            
    if (all(greaterThan(gl_TexCoord[0].xy, vec2(OneNStep, OneNStep))) && 
        all(lessThan(gl_TexCoord[0].xy, vec2(OneMinusOneNStep, OneMinusOneNStep))))
        
        gl_FragColor = (texture2D(x0, gl_TexCoord[0].xy) + a * (
                        texture2D(x,  gl_TexCoord[0].xy + vec2(-OneNStep, 0.0)) + 
                        texture2D(x,  gl_TexCoord[0].xy + vec2( OneNStep, 0.0)) + 
                        texture2D(x,  gl_TexCoord[0].xy + vec2(0.0, -OneNStep)) + 
                        texture2D(x,  gl_TexCoord[0].xy + vec2(0.0,  OneNStep))  
                       )) / c;
    else
        gl_FragColor = texture2D(x, gl_TexCoord[0].xy);    
}
