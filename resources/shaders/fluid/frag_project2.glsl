/*
    frag_project2.glsl
    Second part of Jos' projection routine
 
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

uniform sampler2D u0;
uniform sampler2D p;
uniform float OneNStep;
uniform float N;

void main()
{     
    //_u[IX(i, j)] -= 0.5f * N * (_p[IX(i + 1, j)] - _p[IX(i - 1, j)]);

    gl_FragColor = texture2D(u0, gl_TexCoord[0].xy) - (
                                                       
                    vec4(0.5, 0.0, 0.0, 0.0) * N *
    
                    (texture2D(p, gl_TexCoord[0].xy + vec2( OneNStep, 0.0)) -
                     texture2D(p, gl_TexCoord[0].xy + vec2(-OneNStep, 0.0))
                    ));
}
