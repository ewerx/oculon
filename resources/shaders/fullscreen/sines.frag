uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec4      iColor1;
uniform vec4      iColor2;

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
    
    color += calcSine(uv, 0.20, 0.15, 0.0, 0.5, vec3(0.0, 0.0, 1.0), 0.2, 15.0);
    color += calcSine(uv, 0.40, 0.15, 0.0, 0.5, vec3(0.0, 1.0, 1.0), 0.1, 17.0);
    color += calcSine(uv, 0.60, 0.15, 0.0, 0.5, vec3(0.5, 0.8, 1.0), 0.05, 23.0);
    
    color += calcSine(uv, 0.18, 0.07, 0.0, 0.7, vec3(0.0, 0.0, 0.7), 0.2, 15.0);
    color += calcSine(uv, 0.26, 0.07, 0.0, 0.7, vec3(0.0, 0.6, 0.7), 0.1, 17.0);
    color += calcSine(uv, 0.46, 0.07, 0.0, 0.7, vec3(0.2, 0.4, 0.7), 0.05, 23.0);
    
    color += calcSine(uv, 0.58, 0.05, 0.0, 0.3, vec3(0.0, 0.0, 0.7), 0.2, 15.0);
    color += calcSine(uv, 0.34, 0.05, 0.0, 0.3, vec3(0.0, 0.6, 0.7), 0.1, 17.0);
    color += calcSine(uv, 0.52, 0.05, 0.0, 0.3, vec3(0.2, 0.4, 0.7), 0.05, 23.0);
    
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