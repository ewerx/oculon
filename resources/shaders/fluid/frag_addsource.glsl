/*
    frag_addsource.glsl
 
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
uniform sampler2D s;
uniform float dt;

void main()
{
	vec4 XPixel = texture2D(x, gl_TexCoord[0].xy);
    vec4 SPixel = texture2D(s, gl_TexCoord[0].xy);

    XPixel += dt * SPixel;
    
	gl_FragColor = XPixel;
}
