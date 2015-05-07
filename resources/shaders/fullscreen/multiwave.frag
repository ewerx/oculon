uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/4ljGD1

float squared(float value)
{
    return value * value;
}

float getAmp(float frequency)
{
    return texture2D(iChannel1, vec2(frequency / 512.0, 0.25)).x;
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

    for (float i = 0.0; i < 5.0; i++)
    {
        uv.y += ( 0.2 * sin(uv.x + i / 7.0 - iGlobalTime * 0.6));
        float Y = uv.y + getWeight(squared(i) * 20.0) *
            ( texture2D(iChannel0, vec2(uvTrue.x, 1)).x - 0.5 );
        lineIntensity = 0.4 + squared(1.6 * abs(mod(uvTrue.x + i / 1.3 + iGlobalTime, 2.0) - 1.0));
        glowWidth = abs(lineIntensity / ( 150.0 * Y ));
        color += vec3(glowWidth * ( 2.0 + sin(iGlobalTime * 0.13)),
                      glowWidth * ( 2.0 - sin(iGlobalTime * 0.23)),
                      glowWidth * ( 2.0 - cos(iGlobalTime * 0.19)));
    }

    gl_FragColor = vec4(color, 1.0);
}



