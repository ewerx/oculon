#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
//uniform sampler2D iChannel0;
//uniform sampler2D iChannel1;
uniform vec2      iMouse;

#define time iGlobalTime
#define resolution iResolution
#define mouse iMouse

// https://www.shadertoy.com/view/XsXXDN
#define M_PI 3.14159265358979323846
#define M_PHI 0.6180339887498949

#define ITERS 192
#define CYCL 5
#define CYCH 8
#define A_SPEED 1.0

void main(void)
{
	vec2 z0 = gl_FragCoord.xy / (iResolution.xy / 4.0) - 2.0;
	z0.x *= iResolution.x / iResolution.y;
	vec2 z = z0;
	
	vec2 c = iMouse.xy / (iResolution.xy / 4.0) - 2.0;
	c.x *= iResolution.x / iResolution.y;
	c += 0.01 * vec2(sin(iGlobalTime), sin(iGlobalTime * M_PHI));
	
    
	vec3 col0 = vec3(0.6, 0.4, 0.0);
	vec3 col1 = vec3(0.0, 0.6, 0.8);
	vec3 col2 = vec3(0.4, 0.0, 0.2);
	
	vec2 acc = vec2(0.0, 0.0);
	vec2 last[CYCH];
	
	for (int i = 0; i < CYCH; ++i)
		last[i] = vec2(0.0, 0.0);
	
	for (int i = 0; i < ITERS; ++i) {
		float zm2 = dot(z, z);
		acc += z / zm2;
		if (ITERS - i <= CYCH)
			last[ITERS - i - 1] = z / zm2;
		if (zm2 > 1e6) {
			col0 = vec3(0.8, 0.4, 0.0);
			col1 = vec3(0.0, 0.4, 0.8);
			col2 = vec3(0.2, 0.2, 0.2);
			break;
		}
		z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y);
   	    z += c;
	}
	
	for (int i = 1; i < CYCH; ++i)
		last[i] += last[i-1];
	vec2 a2 = acc / dot(acc, acc);
	for (int i = CYCL; i <= CYCH; ++i) {
		vec2 stuff = acc;
		if (ITERS / i * i != ITERS)
			stuff -= last[ITERS - ITERS / i * i - 1];
		stuff -= last[i - 1] * float(ITERS / i);
		a2 += stuff / dot(stuff, stuff);
	}
	
	float rad = -log(length(a2));
	float ang = atan(a2.y, a2.x) / (2.0 * M_PI);
	
	gl_FragColor = vec4(fract(rad + iGlobalTime * M_PHI) * col0 +
						fract(ang * 6.0 + iGlobalTime) * col1 +
						fract(rad + ang * 7.0 - iGlobalTime * 7.0/9.0) * col2,
						1.0);
}