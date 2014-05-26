#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float     iScale;
uniform float     iFrequencyX;
uniform float     iFrequencyY;
uniform vec4      iColor;

// Lissajous curve
// http://en.wikipedia.org/wiki/Lissajous_curve

const float PI = 3.14159265358979323;
const int steps = 256;

vec2 lissajous(float t, float a, float b, float d)
{
	return vec2(sin(a*t+d), sin(b*t));
}

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy)*2.0-1.0;
	uv.x *= iResolution.x / iResolution.y;
//	vec2 mouse = iMouse.xy / iResolution.xy;
    
	float a = iFrequencyX;
	float b = iFrequencyY;
	float d = iGlobalTime;	// phase
	
//	if (iMouse.z > 0.0) {
//		a = 1.0 + mouse.x*3.0;
//		b = 1.0 + mouse.y*3.0;
//	}
	
	float m = 1.0;
	float period = PI*2.0;
    vec2 lp = lissajous(iGlobalTime, a, b, d) * iScale;
    for(int i = 1; i <= steps; i++)
    {
        float t = float(i)*period / float(steps);
		t += iGlobalTime;
        vec2 p = lissajous(t, a, b, d) * iScale;
		
		// distance to line
        vec2 pa = uv - p;
        vec2 ba = lp - p;
        float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
        vec2 q = pa - ba*h;
        m = min( m, dot( q, q ) );
		
        lp = p;
    }
    m = sqrt( m );
	m = smoothstep(0.01, 0.0, m);
	
	gl_FragColor = mix(vec4(0.0), iColor, m);
}
