uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/llSGDh#

void main()
{
    // create pixel coordinates
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
    // first texture row is frequency data
    float fft  = texture2D( iChannel1, vec2(uv.x,0.25) ).x;
    
    // second texture row is the sound wave
    float wave = texture2D( iChannel1, vec2(uv.x,0.75) ).x;
    
    // convert frequency to colors
    vec3 col = vec3(1.0)*fft;
    
    // add wave form on top
    col += 1.0 -  smoothstep( 0.0, 0.01, abs(wave - uv.y) );
    
    col = pow( col, vec3(1.0,0.5,2.0) );
    
    // output final color
    gl_FragColor = vec4(col,1.0);
}