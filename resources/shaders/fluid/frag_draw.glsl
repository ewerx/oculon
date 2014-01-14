/*
    frag_draw.glsl
 
    Map densities from the fluid model to gradient colors and optionally light them
 
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

uniform sampler2D Density;              //texture containing the density map
uniform sampler2D GradientColors;       //texture of 1 pixel height and a width corresponding to the GradientColorCount containing all gradient colors
uniform float LightingFactor;           //magic number of phagor's algorithm
uniform float LightingPower;            //magic number of phagor's algorithm
uniform float OnePixel;                 //width/height of the quadratic density texture in pixels
uniform bool Show3DLighting;            //turn on/off phagor's 3D lighting scheme

//lighting calculation
float az, bz, nz, w;

void main()
{   
    //the density texture uses only the red channel to store the density values
	float densityvalue = texture2D(Density, gl_TexCoord[0].xy).r;
    
    //clamp/range densityvalue between 0..1 to get the color index inside the gradient's texture
    //(all coordinates are in 0..1 float format)
    vec2 colorposition = vec2(clamp(densityvalue, 0.0, 1.0), 0.0);
       
    //sample density dependend color from gradient's texture
    vec4 color = texture2D(GradientColors, colorposition);
    
    //brighten current pixel's color according to a virtual light source
    if (Show3DLighting)
    {
        az = densityvalue - texture2D(Density, gl_TexCoord[0].xy + vec2(OnePixel, 0.0)).r;
        bz = densityvalue - texture2D(Density, gl_TexCoord[0].xy + vec2(0.0, OnePixel)).r;        
        nz = 1.0 / length(vec3(az, bz, 1.0));

        w = (colorposition.x * LightingFactor) * pow(nz, LightingPower);

        color += vec4(w, w, w, 0.0);       
        color = min(color, 1.0);
    }
    
	gl_FragColor.rgb = color.rgb;
    gl_FragColor.a = 1.0;
}
