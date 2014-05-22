#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)

uniform vec4    iColor1;
uniform vec4    iColor2;
uniform int     iIterations;
uniform float   iDensity;
uniform float   iFOV;
uniform float   iNoiseLevel;
uniform float   iTunnelWidth;

// https://www.shadertoy.com/view/ldfXzN

#define speed iGlobalTime

float hash(float n)
{
	return fract(sin(n)*43758.5453123);
}

float noise3(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(iNoiseLevel-(iNoiseLevel-1.0)*f);
    float n = p.x + p.y*57.0 + p.z*113.0;
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

float sdCylinder( vec3 p, vec3 c )
{
    return length(p.xy-c.xy)-c.z;
}

vec2 rot(vec2 k, float t)
{
    return vec2(cos(t)*k.x-sin(t)*k.y,sin(t)*k.x+cos(t)*k.y);
}

float DE(vec3 p)
{
    p.z+=speed*2.0;
    p.x+=sin(p.z*0.5)*2.0;
    return sdCylinder(p, vec3(0.0,0.0,iTunnelWidth));
}

vec4 DEc4(vec3 p)
{
    float t=DE(p);
    p.z+=speed*4.0;
    t+=noise3(p*1.75-speed*1.5)*0.6;
    
    vec4 res = vec4(  clamp( t, 0.0, 1.0 ) );
    res.xyz = mix( vec3(1.0,1.0,1.0), vec3(0.0,0.0,0.05), res.x );
	return res;
}

void main( void )
{
	vec3 ro=vec3(0.0, 0.0, -3.0);
	vec3 rd=normalize( vec3( (-(iFOV/2.0)+iFOV*gl_FragCoord.xy/iResolution.xy)*vec2(iResolution.x/iResolution.y,1.0), 1.0));
	vec3 lig=normalize(vec3(0.0, 1.0, 0.0));
    
    ro.x+=cos(speed)*2.5;
    rd.xy=rot(rd.xy,speed*0.5+cos(speed));
    rd.x+=sin(speed-3.14159*0.5)*0.5;
    
    
	float d=0.0;
	vec4 col = vec4(iColor1.xyz, 0.0);
	
	for(int i=0; i < iIterations; i++)
    {
		vec4 res=DEc4(ro+rd*d);
        res.w *= iDensity;
        res.xyz *= res.w;
        col = col + res*(1.0 - col.w);
        d+=0.1;
	}
    
    col.xyz/=col.w;
    col = sqrt( col );
    
	gl_FragColor = vec4( col.xyz, iColor1.a );
}