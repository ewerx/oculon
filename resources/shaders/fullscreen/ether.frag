uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
//uniform sampler2D iChannel0;
//uniform sampler2D iChannel1;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform float     iMaxGlow;
uniform int       iLayers;
uniform float     iDepth;
uniform float     iIntensity;
uniform float     iDeformation;

// https://www.shadertoy.com/view/MsjSW3

//Ether by nimitz (twitter: @stormoid)

#define t iGlobalTime
mat2 m(float a)
{
    float c = cos(a), s = sin(a);
    return mat2(c, -s, s, c);
}

float map(vec3 p)
{
    p.xz *= m(t * 0.4); // xz rotate
    p.xy *= m(t * 0.3); // zy rotate
    vec3 q = p * iDeformation + t * 1.;
    return length(p + vec3(sin(t * 0.7))) * log(length(p) + 1.) + sin(q.x + sin(q.z + sin(q.y))) * 0.5 - 1.;
}

void main()
{
    vec2 p = gl_FragCoord.xy / iResolution.y - vec2(.9, .5); // slight off center
    vec3 cl = vec3(0.);
    float d = 2.5; // blur
    for (int i = 0; i <= iLayers; i++)
    {
        vec3 p = vec3(0, 0, iDepth) + normalize(vec3(p, -1.)) * d;
        float rz = map(p);
        float f = clamp(( rz - map(p + .1)) * 0.5, -.1, 1.);
        vec3 l = iColor2.xyz + iColor1.xyz * f * iIntensity;//vec3(0.1, 0.3, .4) + vec3(5., 2.5, 3.) * f;
        cl = cl * l + ( 1. - smoothstep(0., iMaxGlow, rz)) * .7 * l;
        d += min(rz, 1.);
    }

    gl_FragColor = vec4(cl, 1.);
}
