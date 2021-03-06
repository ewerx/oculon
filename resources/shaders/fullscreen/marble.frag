uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
//uniform sampler2D iChannel0;
//uniform sampler2D iChannel1;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform float     iDensity;
uniform float     iZoom;
uniform vec2      iNoiseMap;

// https://www.shadertoy.com/view/4tBGWR

float hash( vec2 p )
{
    float h = dot(p,vec2(127.1,311.7));
    
    return -1.0 + 2.0*fract(sin(h)*43758.5453123);
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
    vec2 zoomCoord = gl_FragCoord.xy * iZoom;
    float v = 0.5 + 0.5 * sin(zoomCoord.x + zoomCoord.y + t(zoomCoord+iGlobalTime));
    gl_FragColor = vec4(v * iColor1.xyz, 1.0) + (iColor2 * 0.25);//vec4(v+.1,v,v,1.0);
}