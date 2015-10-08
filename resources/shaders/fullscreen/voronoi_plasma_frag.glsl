#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/XljSWw
vec3 hash33(vec3 p){
    float n = sin(dot(p, vec3(7, 157, 113)));
    return fract(vec3(2097152, 262144, 32768)*n);
}

float voronoi(vec3 p){
    
    vec3 b, r, g = floor(p);
    p = fract(p); // "p -= g;" works on some GPUs, but not all, for some annoying reason.
    float d = 1.;
    
    for(float j = -1.; j < 1.01; j++) {
        for(float i = -1.; i < 1.01; i++) {
            
            b = vec3(i, j, -1.);
            r = b - p + hash33(g+b);
            d = min(d, dot(r,r));
            
            b.z = 0.0;
            r = b - p + hash33(g+b);
            d = min(d, dot(r,r));
            
            b.z = 1.;
            r = b - p + hash33(g+b);
            d = min(d, dot(r,r));
            
        }
    }
    
    return d; // Range: [0, 1]
}

vec3 hsv2rgb (in float h, in float s, in float v) {
    return v * (1.0 + 0.5 * s * (cos (2.0 * 3.1415926535898 * (h + vec3 (0.0, 2.0 / 3.0, 1.0 / 3.0))) - 1.0));
}


float noiseLayers(in vec3 p) {
    vec3 t = vec3(0., 0., p.z+sin(iGlobalTime*.25));
    
    const int iter = 5; // Just five layers is enough.
    float tot = 0., sum = 0., amp = 1.; // Total, sum, amplitude.
    
    for (int i = 0; i < iter; i++) {
        tot += voronoi(p + t) * amp; // Add the layer to the total.
        p *= 2.0; // Position multiplied by two.
        t *= 1.5; // Time multiplied by less than two.
        sum += amp; // Sum of amplitudes.
        amp *= 0.5; // Decrease successive layer amplitude, as normal.
    }
    
    return tot/sum; // Range: [0, 1].
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - iResolution.xy*0.5) / iResolution.x;
    //uv.y *= iResolution.x/iResolution.y;
    
    float light = smoothstep (-0.7, 0.7, cos (cos(iGlobalTime*1.2)));
    
    vec3 rd = normalize(vec3(uv.x, uv.y, 3.1415926535898/8.));
    
    float cs = cos(iGlobalTime*0.125), si = sin(iGlobalTime*0.125);
    rd.xy *= mat2(cs, -si, si, cs);
    
    float c = noiseLayers(rd*.7);
    
    c *= sqrt(c*(1.-length(uv)))+sin(1.-length(uv))*2.;
    vec3 col = vec3(c);
    
    vec3 col2 =  hsv2rgb(length(uv) * 0.6 + iGlobalTime * .5, 0.9, light);
    col = mix(col, col.xyz*.3+c*.86, (rd.x*rd.y)*.45);
    col *= mix(col, col2, 1.-length(uv));
    
    gl_FragColor = vec4(clamp(col, 0., 1.), 1.);
}
