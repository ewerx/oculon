/*
    frag_setbnd.glsl
 
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
uniform int b;
uniform float OneNStep;

void main()
{
/*    
     for (int i = 1 ; i <= N ; i++)
     {
        x[IX(0  , i)] = (b == 1) ? -x[IX(1, i)] : x[IX(1, i)];
        x[IX(N+1, i)] = (b == 1) ? -x[IX(N, i)] : x[IX(N, i)];
        x[IX(i  , 0)] = (b == 2) ? -x[IX(i, 1)] : x[IX(i, 1)];
        x[IX(i, N+1)] = (b == 2) ? -x[IX(i, N)] : x[IX(i, N)];
     }
     
     x[IX(0  , 0  )] = 0.5f * (x[IX(1, 0  )] + x[IX(0  , 1)]);
     x[IX(0  , N+1)] = 0.5f * (x[IX(1, N+1)] + x[IX(0  , N)]);
     x[IX(N+1, 0  )] = 0.5f * (x[IX(N, 0  )] + x[IX(N+1, 1)]);
     x[IX(N+1, N+1)] = 0.5f * (x[IX(N, N+1)] + x[IX(N+1, N)]);
 */   
    
    gl_FragColor = texture2D(x, gl_TexCoord[0].xy);
    
    //x[IX(0  , i)] = (b == 1) ? -x[IX(1, i)] : x[IX(1, i)];
    if (gl_TexCoord[0].x == 0.0 && gl_TexCoord[0].y > 0.0 && gl_TexCoord[0].y < 1.0)
        gl_FragColor = (b == 1) ? -texture2D(x, vec2(OneNStep, gl_TexCoord[0].y)) : texture2D(x, vec2(OneNStep, gl_TexCoord[0].y));
    
    //x[IX(N+1, i)] = (b == 1) ? -x[IX(N, i)] : x[IX(N, i)];
    else if (gl_TexCoord[0].x == 1.0 && gl_TexCoord[0].y > 0.0 && gl_TexCoord[0].y < 1.0)
        gl_FragColor = (b == 1) ? -texture2D(x, vec2(1.0 - OneNStep, gl_TexCoord[0].y)) : texture2D(x, vec2(1.0 - OneNStep, gl_TexCoord[0].y));
    
    //x[IX(i  , 0)] = (b == 2) ? -x[IX(i, 1)] : x[IX(i, 1)];
    if (gl_TexCoord[0].y == 0.0 && gl_TexCoord[0].x > 0.0 && gl_TexCoord[0].x < 1.0)
        gl_FragColor = (b == 2) ? -texture2D(x, vec2(gl_TexCoord[0].x, OneNStep)) : texture2D(x, vec2(gl_TexCoord[0].x, OneNStep));
    
    //x[IX(i, N+1)] = (b == 2) ? -x[IX(i, N)] : x[IX(i, N)];    
    else if (gl_TexCoord[0].y == 1.0  && gl_TexCoord[0].x > 0.0 && gl_TexCoord[0].x < 1.0)
        gl_FragColor = (b == 2) ? -texture2D(x, vec2(gl_TexCoord[0].x, 1.0 - OneNStep)) : texture2D(x, vec2(gl_TexCoord[0].x, 1.0 - OneNStep));
    
    //x[IX(0  , 0  )] = 0.5f * (x[IX(1, 0  )] + x[IX(0  , 1)]);
    //x[IX(0  , N+1)] = 0.5f * (x[IX(1, N+1)] + x[IX(0  , N)]);
    if (gl_TexCoord[0].x == 0.0)
    {
        if (gl_TexCoord[0].y == 0.0)
            gl_FragColor = 0.5 * (texture2D(x, vec2(OneNStep, 0.0)) + texture2D(x, vec2(0.0, OneNStep)));    
        else if (gl_TexCoord[0].y == 1.0)
            gl_FragColor == 0.5 * (texture2D(x, vec2(OneNStep, 1.0)) + texture2D(x, vec2(0.0, 1.0 - OneNStep)));
    }

    //x[IX(N+1, 0  )] = 0.5f * (x[IX(N, 0  )] + x[IX(N+1, 1)]);
    //x[IX(N+1, N+1)] = 0.5f * (x[IX(N, N+1)] + x[IX(N+1, N)]);
    else if (gl_TexCoord[0].x == 1.0)
    {
        if (gl_TexCoord[0].y == 0.0)
            gl_FragColor = 0.5 * (texture2D(x, vec2(1.0 - OneNStep, 0)) + texture2D(x, vec2(1.0, OneNStep)));
        else if (gl_TexCoord[0].y == 1.0)
            gl_FragColor = 0.5 * (texture2D(x, vec2(1.0 - OneNStep, 1.0)) + texture2D(x, vec2(1.0, 1.0 - OneNStep)));
    }
}
