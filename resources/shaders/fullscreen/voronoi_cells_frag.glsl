#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform float   iGap;
uniform float   iAgitation;
uniform float   iZoom;

// https://www.shadertoy.com/view/4tsXRH
// Created by David Bargo - davidbargo/2015
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Comment to generate a static but faster image
#define ANIMATE

void main()
{
    vec2 q = gl_FragCoord.xy/iResolution.yy;
    q.x += iAgitation;
    vec2 p = iZoom*q*mat2(0.7071, -0.7071, 0.7071, 0.7071);
    
    vec2 pi = floor(p);
    vec4 v = vec4( pi.xy, pi.xy + 1.0);
    v -= 64.*floor(v*0.015);
    v.xz = v.xz*1.435 + 34.423; // more agitation
    v.yw = v.yw*2.349 + 183.37; // more agitation
    v = v.xzxz*v.yyww;
    v *= v;
    
    v *= iGlobalTime*0.000004 + 0.5;
    vec4 vx = iGap*sin(fract(v*0.00047)*6.2831853);
    vec4 vy = iGap*sin(fract(v*0.00074)*6.2831853);
    
    vec2 pf = p - pi;
    vx += vec4(0., 1., 0., 1.) - pf.xxxx;
    vy += vec4(0., 0., 1., 1.) - pf.yyyy;
    v = vx*vx + vy*vy;
    
    v.xy = min(v.xy, v.zw);
    vec3 col = mix(vec3(0.0,0.4,0.9), vec3(0.0,0.95,0.9), min(v.x, v.y) );
    gl_FragColor = vec4(col, 1.0);
}
