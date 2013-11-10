uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;       // audio texture
uniform float     iSeparation;
uniform float     iDetail;
uniform int       iStrands;
uniform float     iScale;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform bool      iColorSep;
uniform bool      iBackgroundFlash;
uniform float     iThickness;

// based on https://www.shadertoy.com/view/Xsf3WB
const float tau = 6.28318530717958647692; // 2*PI

void main(void)
{
	vec2 uv = (gl_FragCoord.xy - iResolution.xy*.5)/iResolution.x;
    
	uv = vec2(abs(atan(uv.x,uv.y)/(.5*tau)),length(uv));
    
	// adjust frequency to look pretty
	uv.x *= iDetail;
	
    float seperation = iSeparation;
    
	vec3 wave = vec3(0.0);
	int n = iStrands;
	for ( int i=0; i < n; i++ )
	{
//         float u = uv.x*255.0;
//         float f = fract(u);
//         f = f*f*(3.0-2.0*f);
//         u = floor(u);
//         float sound = mix( texture2D( iChannel0, vec2((u+.5)/256.0,1.0) ).x, texture2D( iChannel0, vec2((u+1.5)/256.0,1.0) ).x, f );
		float sound = texture2D( iChannel0, vec2(uv.x,1.0) ).x;
		
		// choose colour from spectrum
		float a = .9*float(i)*tau/float(n)-.6;
		vec3 phase;
        if (iColorSep) {
            phase = smoothstep(-1.0,.5,vec3(cos(a),cos(a-tau/3.0),cos(a-tau*2.0/3.0)));
        } else {
            float g = cos(a-tau*2.0/3.0);
            phase = smoothstep(-1.0,.5,vec3(g));
        }
		
		wave += phase*smoothstep(iThickness*0.1, 0.0, abs(uv.y - sound*iScale*.3));
		uv.x += seperation/float(n);
	}
	wave *= 3.0/float(n);
    
    // background
	vec4 col = iColor2;
    if (iBackgroundFlash) {
        col = vec4(0);
        col.w = 1.0;
        col.z  += texture2D( iChannel0, vec2(.000,.25) ).x;
        col.zy += texture2D( iChannel0, vec2(.125,.25) ).xx*vec2(1.5,.5);
        col.zy += texture2D( iChannel0, vec2(.250,.25) ).xx;
        col.zy += texture2D( iChannel0, vec2(.375,.25) ).xx*vec2(.5,1.5);
        col.y  += texture2D( iChannel0, vec2(.500,.25) ).x;
        col.yx += texture2D( iChannel0, vec2(.625,.25) ).xx*vec2(1.5,.5);
        col.yx += texture2D( iChannel0, vec2(.750,.25) ).xx;
        col.yx += texture2D( iChannel0, vec2(.875,.25) ).xx*vec2(.5,1.5);
        col.x  += texture2D( iChannel0, vec2(1.00,.25) ).x;
        col *= iColor2;
        // vignetting
        col *= smoothstep( 1.2, 0.0, uv.y );
	}
	
	gl_FragColor = vec4(wave,(wave.x+wave.y+wave.z)) + col;
}