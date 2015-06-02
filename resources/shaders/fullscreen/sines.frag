uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform float     iFreq1;
uniform float     iFreq2;
uniform float     iFreq3;
uniform float     iAmp1;
uniform float     iAmp2;
uniform float     iAmp3;
uniform float     iPower1;
uniform float     iPower2;
uniform float     iPower3;

// https://www.shadertoy.com/view/Xlj3Rm

const float speed = 0.8;
const float widthFactor = 4.0;

vec3 calcSine(vec2 uv,
              float frequency, float amplitude, float shift, float offset,
              vec3 color, float width, float exponent)
{
    float angle = iGlobalTime * speed * frequency + (shift + uv.x) * 6.2831852;
    
    float y = sin(angle) * amplitude + offset;
    
    float scale = pow(smoothstep(width * widthFactor, 0.0, distance(y, uv.y)), exponent);
    
    return color * scale;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec3 color = vec3(0.0);
    
    float freq11 = iFreq1 * 0.5;
    float freq12 = iFreq1 * 1.5;
    
    float freq21 = iFreq2 * 0.6666;
    float freq22 = iFreq2 * 1.6666;
    
    float freq31 = iFreq3 * 1.7;
    float freq32 = iFreq3 * 1.5;
    
    float pow10 = iPower1 * 20.0;
    float pow11 = pow10 * 0.75;
    float pow12 = pow10 * 1.25;
    
    float pow20 = iPower2 * 20.0;
    float pow21 = pow10 * 0.75;
    float pow22 = pow10 * 1.25;
    
    float pow30 = iPower3 * 20.0;
    float pow31 = pow30 * 0.75;
    float pow32 = pow30 * 1.25;
    
    color += calcSine(uv, freq11, iAmp1, 0.0, 0.5, iColor1.xyz*0.6, 0.2, pow10);
    color += calcSine(uv, iFreq1, iAmp1, 0.0, 0.5, iColor2.xyz*0.5, 0.1, pow11);
    color += calcSine(uv, freq12, iAmp1, 0.0, 0.5, iColor1.xyz*0.4, 0.05, pow12);
    
    color += calcSine(uv, freq21, iAmp2, 0.0, 0.7, iColor1.xyz*0.6, 0.2, pow20);
    color += calcSine(uv, iFreq2, iAmp2, 0.0, 0.7, iColor2.xyz*0.5, 0.1, pow21);
    color += calcSine(uv, freq22, iAmp2, 0.0, 0.7, iColor1.xyz*0.4, 0.05, pow22);
    
    color += calcSine(uv, freq31, iAmp3, 0.0, 0.3, iColor1.xyz*0.6, 0.2, pow30);
    color += calcSine(uv, iFreq3, iAmp3, 0.0, 0.3, iColor2.xyz*0.5, 0.1, pow31);
    color += calcSine(uv, freq32, iAmp3, 0.0, 0.3, iColor1.xyz*0.4, 0.05, pow32);
    
    gl_FragColor = vec4(color,1.0);
}

// alt sines: https://www.shadertoy.com/view/MtB3zw

//const float speed = 1.5;
//const float widthFactor = 4.0;
//
//vec3 calcSine(vec2 uv,
//              float frequency, float amplitude, float shift, float offset,
//              vec3 color, float width, float exponent)
//{
//    float angle = iGlobalTime * speed * frequency + (shift + uv.x) * 6.2831852;
//    
//    float y = sin(angle) * amplitude + offset;
//    
//    float scale = pow(smoothstep(width * widthFactor, 0.0, distance(y, uv.y)), exponent);
//    
//    return color * scale;
//}
//
//void mainImage( out vec4 fragColor, in vec2 fragCoord )
//{
//    vec2 uv = fragCoord.xy / iResolution.xy;
//    vec3 color = vec3(0.0);
//    
//    color += calcSine(uv, 2.0, 0.45, 0.0, 0.5, vec3(0.0, 0.0, 1.0), 0.1, 3.0);
//    color += calcSine(uv, 2.2, 0.45, 0.2, 0.5, vec3(0.0, 1.0, 0.0), 0.1, 3.0);
//    color += calcSine(uv, 2.4, 0.45, 0.4, 0.5, vec3(1.0, 0.0, 0.0), 0.1, 3.0);
//    
//    color += calcSine(uv, 2.6, 0.4, 0.6, 0.5, vec3(0.5, 0.0, 1.0), 0.1, 4.0);
//    color += calcSine(uv, 2.8, 0.4, 0.8, 0.5, vec3(1.0, 0.5, 1.0), 0.1, 4.0);
//    color += calcSine(uv, 3.0, 0.4, 0.0, 0.5, vec3(1.0, 0.0, 0.5), 0.1, 4.0);
//    
//    color += calcSine(uv, 2.25, 0.45, 0.5, 0.5, vec3(0.0, 0.5, 1.0), 0.1, 4.0);
//    color += calcSine(uv, 2.50, 0.45, 0.5, 0.5, vec3(0.5, 1.0, 0.5), 0.1, 4.0);
//    color += calcSine(uv, 2.75, 0.45, 0.5, 0.5, vec3(1.0, 0.5, 0.0), 0.1, 4.0);
//    
//    //this "takes the edge off" the overlay
//    //color *= 0.5;
//    
//    //this creates a solarized effect
//    //color = smoothstep(0.2, 0.5, color);
//    
//    fragColor = vec4(color,1.0);
//}

// alt sines 2: https://www.shadertoy.com/view/ltB3zw

//vec3 calcSine(vec2 uv,
//              float frequency, float amplitude, float shift, float offset,
//              vec3 color, float width, float exponent)
//{
//    float y = sin(iGlobalTime * frequency + shift + uv.x) * amplitude + offset;
//    float scale = pow(smoothstep(width, 0.0, distance(y, uv.y)), exponent);
//    return color * scale;
//}
//
//void mainImage( out vec4 fragColor, in vec2 fragCoord )
//{
//    vec2 uv = fragCoord.xy / iResolution.xy;
//    vec3 color = vec3(0.0);
//    
//    color += calcSine(uv, 2.0, 0.25, 0.0, 0.5, vec3(0.0, 0.0, 1.0), 0.3, 1.0);
//    color += calcSine(uv, 2.6, 0.25, 0.2, 0.5, vec3(0.0, 1.0, 0.0), 0.3, 1.0);
//    color += calcSine(uv, 2.9, 0.25, 0.4, 0.5, vec3(1.0, 0.0, 0.0), 0.3, 1.0);
//    
//    fragColor = vec4(color,1.0);
//}