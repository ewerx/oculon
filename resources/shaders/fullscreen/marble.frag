uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform vec3      iMouse;
uniform float     iDensity;

// https://www.shadertoy.com/view/4tBGWR

//#define SLOWMOTION
//#define MOD_KazimirO

float hash( vec2 p )
{
    float h = dot(p,vec2(127.1,311.7));
    
#ifdef MOD_KazimirO
    return -1.0 + 2.0*fract(sin(h)*0.0437585453123*iGlobalTime);
#else
    return -1.0 + 2.0*fract(sin(h)*43758.5453123);
#endif
}

float noise( in vec2 p )
{
    vec2 i = floor( p );
    vec2 f = fract( p );
    
    vec2 u = f*f*(iDensity-(iDensity-1.0)*f);
    
    return mix( mix( hash( i + vec2(0.0,0.0) ),
                    hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ),
                    hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float t( in vec2 coord )
{
    float
    value = noise(coord / 64.) * 64.;
    value += noise(coord / 32.) * 32.;
    value += noise(coord / 16.) * 16.;
    value += noise(coord / 8.) * 8.;
    value += noise(coord / 4.) * 4.;
    value += noise(coord / 2.) * 2.;
    value += noise(coord);
    value += noise(coord / .5) * .5;
    value += noise(coord / .25) * .25;
    return value;
}

void main()
{
    float timeScale=
#ifdef SLOWMOTION
    0.1;
#else
    1.0;
#endif
    vec2 zoomCoord = gl_FragCoord.xy * .1;
    float v=0.5+0.5*sin(zoomCoord.x+zoomCoord.y+t(zoomCoord+iGlobalTime*timeScale));
    gl_FragColor = vec4(v * iColor1.xyz, 1.0) + (iColor2 * 0.1);//vec4(v+.1,v,v,1.0);
}