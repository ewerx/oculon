uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec4      iColor1;
uniform vec4      iColor2;

// https://www.shadertoy.com/view/lsXSzH
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// I've not seen anybody out there computing correct cell interior distances for Voronoi
// patterns yet. That's why they cannot shade the cell interior correctly, and why you've
// never seen cell boundaries rendered correctly.
//
// However, here's how you do mathematically correct distances (note the equidistant and non
// degenerated grey isolines inside the cells) and hence edges (in yellow):
//
// http://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm


#define ANIMATE

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

vec2 hash( vec2 p )
{
    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
    return fract(sin(p)*43758.5453);
}


vec2 rotate(vec2 v, float alpha)
{
    float vx = v.x*cos(alpha)-v.y*sin(alpha);
    float vy = v.x*sin(alpha)+v.y*cos(alpha);
    v.x = vx;
    v.y = vy;
    return v;
}

float particles( in vec2 x )
{
    vec2 n = floor(x);
    vec2 f = fract(x);
    
    float d = 0.0;
    for( int i=0; i<=15; i++ )
    {
        vec2 particle = hash(float(i)*vec2(0.323,0.156));
        
        particle += 0.05*sin(1.0*iGlobalTime + 6.2831*particle );
        
        float w = 3.0*hash(float(i)*0.254);
        d += w/length(particle-x);
    }
    
    return d;
}

float sinm(float t, float vmin, float vmax)
{
    return (vmax-vmin)*0.5*sin(t)+vmin+(vmax-vmin)*0.5;
}

void main()
{
    vec2 p = gl_FragCoord.xy/iResolution.xx;
    
    float c = particles( p );
    float s = 0.0015*c + 1.0*sin(40.0*log(c)) - 2.0*1.5;
    
    vec3 col = (0.5 + 0.5*s)*vec3(s*0.95321,s*0.135,0.25);
    
    gl_FragColor = vec4(col,1.0);
}
