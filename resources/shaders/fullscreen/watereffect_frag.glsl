#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
//uniform sampler2D iChannel0;
//uniform sampler2D iChannel1;
uniform vec2      iMouse;

#define time iGlobalTime
#define resolution iResolution
#define mouse iMouse

// water turbulence effect by joltz0r 2013-07-04, improved 2013-07-07

//varying vec2 surfacePosition;

#define MAX_ITER 10
void main( void ) {
	vec2 sp =  gl_FragCoord.xy / iResolution.xy * (iResolution.y/iResolution.x);//vec2(.4, 0.5);//surfacePosition
	vec2 p = sp*8.0- vec2(20.0);
	vec2 i = p;
	float c = 1.0;
	float inten = .05;
    
	for (int n = 0; n < MAX_ITER; n++)
	{
		float t = time * (1.0 - (3.0 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(MAX_ITER);
	c = 1.5-sqrt(c);
	gl_FragColor = vec4(vec3(c*c*c*c*c*c*c*c*c*c*c), 19.0) + vec4(0.0, 0.3, 0.5, 1.0) * vec4(1.0, 0.5, 0.5, 1.0);
}