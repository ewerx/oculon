uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec4      iColor2;
uniform int       iNumBands;
uniform float     iGlowWidth;
uniform float     iGlowLength;
uniform float     iIntensity;
uniform float     iWaveRate;
uniform float     iCurvature;
uniform float     iSeparation;

// https://www.shadertoy.com/view/4ljGD1

float squared(float value)
{
    return value * value;
}

float getAmp(float frequency)
{
    return texture2D(iChannel1, vec2(frequency / 512.0, 1.0)).x;
}

float getWeight(float f)
{
    return ( +getAmp(f - 2.0) + getAmp(f - 1.0) + getAmp(f + 2.0) + getAmp(f + 1.0) + getAmp(f)) / 5.0;
}

void main()
{
    vec2 uvTrue = gl_FragCoord.xy / iResolution.xy;
    vec2 uv = -1.0 + 2.0 * uvTrue;

    float lineIntensity;
    float glowWidth;
    vec3 color = vec3(0.0);

    float numBands = float(iNumBands);
    for (float i = 0.0; i < numBands; i++)
    {
        uv.y += ( iSeparation * sin(uv.x + i * iCurvature - iGlobalTime * iWaveRate));
        float Y = uv.y + getWeight(squared(i) * 20.0) *
            ( texture2D(iChannel0, vec2(uvTrue.x, 0.0)).x - 0.5 );
        lineIntensity = iIntensity * 0.25 + squared(iIntensity * abs(mod(uvTrue.x + i * iGlowLength + iGlobalTime, 2.0) - 1.0));
        float glowFactor = 1.0 / (0.02 * iGlowWidth);
        glowWidth = abs(lineIntensity / (glowFactor * Y));
        color += vec3(glowWidth * ( 2.0 + sin(iGlobalTime * iColor2.x)),
                      glowWidth * ( 2.0 + sin(iGlobalTime * iColor2.y)),
                      glowWidth * ( 2.0 + sin(iGlobalTime * iColor2.z)));
    }

    gl_FragColor = vec4(color, 1.0);
}



