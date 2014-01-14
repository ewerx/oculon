/*
    frag_addvect.glsl
 
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

uniform sampler2D d0;
uniform sampler2D u;
uniform sampler2D v;
uniform float OneNStep;
uniform float OneMinusOneNStep;
uniform float DT0;

void main()
{  
    if (all(greaterThan(gl_TexCoord[0].xy, vec2(0.0, 0.0))) && all(lessThan(gl_TexCoord[0].xy, vec2(1.0, 1.0))))
    {            
/*   
        //float x, y, i0, i1, j0, j1, s0, t0, s1, t1;

        x = clamp(gl_TexCoord[0].x - dt0 * texture2D(u, gl_TexCoord[0].xy).r, 0.0, n);
        
        s1 = mod(x, OneNStep);
        s0 = 1.0 - s1;
        i0 = ((x / OneNStep) - s1) * OneNStep;
        i1 = i0 + OneNStep;
            
        y = clamp(gl_TexCoord[0].y - dt0 * texture2D(v, gl_TexCoord[0].xy).r, 0.0, n);

        t1 = mod(y, OneNStep);
        t0 = 1.0 - t1;
        j0 = ((y / OneNStep) - t1) * OneNStep;
        j1 = j0 + OneNStep;
 
        gl_FragColor = s0 * (t0 * texture2D(d0, vec2(i0, j0)) + t1 * texture2D(d0, vec2(i0, j1))) + 
                       s1 * (t0 * texture2D(d0, vec2(i1, j0)) + t1 * texture2D(d0, vec2(i1, j1)));

*/
        
        vec2 uv = vec2(texture2D(u, gl_TexCoord[0].xy).r, texture2D(v, gl_TexCoord[0].xy).r);        
        vec2 xy = clamp(gl_TexCoord[0].xy - DT0 * uv, 0.0, OneMinusOneNStep);
        
        vec2 st1 = mod(xy, OneNStep);
        vec2 st0 = vec2(1.0, 1.0) - st1;
        vec2 ij0 = ((xy / OneNStep) - st1) * OneNStep;
        vec2 ij1 = ij0 + OneNStep;

        gl_FragColor = st0.s * (st0.t * texture2D(d0, ij0) + st1.t * texture2D(d0, vec2(ij0.x, ij1.y))) +
                       st1.s * (st0.t * texture2D(d0, vec2(ij1.x, ij0.y)) + st1.t * texture2D(d0, ij1));
        
   }
    else
        gl_FragColor = texture2D(d0, gl_TexCoord[0].xy);
}

/*
void FluidModelGLSL::advect(GPUBuffer& d, GPUBuffer& d0, int b, GPUBuffer& _u, GPUBuffer& _v)
{
     unsigned int gridsize = GridSize;
     unsigned int n = N;
     int i, j, i0, j0, i1, j1;
     float x, y, s0, t0, s1, t1, dt0;
     
     dt0 = DT * n;
     
     for (i = 1; i <= n; i++)
     for (j = 1; j <= n; j++)
     {
     x = i - dt0 * _u[IX(i, j)];
     y = j - dt0 * _v[IX(i, j)];
     
     x = RANGE(x, 0.5f, n + 0.5f);          
     i0 = (int) x;
     i1 = i0 + 1;
     
     y = RANGE(y, 0.5f, n + 0.5f);           
     j0 = (int) y;
     j1 = j0 + 1;
     
     s1 = x - i0;
     s0 = 1 - s1;
     t1 = y - j0;
     t0 = 1 - t1;
     
     d[IX(i, j)] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) + 
     s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
     }
     
     setBnd(d, b);
 */
