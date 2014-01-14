/*
    frag_addforcearea_circular.glsl
    Add a smooth (faded) circular force area

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

#version 110

uniform sampler2D field0;
uniform float x_center;
uniform float y_center;
uniform float s;
uniform float r;

void main()
{     
    vec4 SourcePixel = texture2D(field0, gl_TexCoord[0].xy);
    gl_FragColor = SourcePixel;

    float cur_x     = gl_TexCoord[0].x;
    float cur_y     = gl_TexCoord[0].y;
                     
    //check, if the current pixel is within a bounding box around the radius
    //for performance reasons do a cascaded "if" instead of "if (a && b && c && d)"
    //because it is not known, if the compiler does a lazy evaluation
                            
    if (cur_x >= clamp(x_center - r, 0.0, 1.0))
        if (cur_x <= clamp(x_center + r, 0.0, 1.0))
            if (cur_y >= clamp(y_center - r, 0.0, 1.0))
                if (cur_y <= clamp(y_center + r, 0.0, 1.0))
                {
                    float dx = x_center - cur_x;
                    float dy = y_center - cur_y;
                    float f  = 1.0 - (sqrt(dx * dx + dy * dy) / r);
                    gl_FragColor = SourcePixel + (clamp(f, 0.0, 1.0) * s);
                }

}
