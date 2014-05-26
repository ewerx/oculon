#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
//uniform sampler2D iChannel0;
//uniform sampler2D iChannel1;
uniform vec2      iMouse;

#define time iGlobalTime
#define resolution iResolution
#define mouse iMouse

void main( void ) {
	vec2 uv = gl_FragCoord.xy / resolution.xy;
	vec2 aspect = vec2( resolution.x / resolution.y, 1.0 );
	vec2 p = ( uv * 2.0 - 1.0 ) * aspect;
	p*=1.0;
	vec2 mt = (mouse * 2.0 - 1.0) * aspect;
	vec3 c = vec3(0); // <-- please initialize, otherwise it's not running on other browsers
	float t2 = time*0.1+200.0;
	
	for(float i = 0.0; i < 100.0; i++)
	{
		float r = cos(i * 1000.0);
		vec2 cp = vec2(r + sin(t2*(r+0.5)*0.5), (cos(t2*0.5*(r+0.5)))*1.1);
		float d = distance(cp, p) / (0.08+distance(cp, mt)*0.1);
		float a = pow(sin(t2*48.0 + r), 0.66);
		float e = smoothstep(-a*0.3, 0.1, 1.0 - d)-0.001;
		c += (e) * mix(vec3(0.0, 0.1, 0.9), vec3(0.0, 0.5, 0.9), a);
	}
	
	c *= 0.5;
	c *= smoothstep(-1.5, 1.0, 1.0 - length(p))*0.9;
	c = pow(c, vec3(0.7, 0.7, 1.0));
	c -= 0.05;
	c+= fract(sin(dot(p, vec2(344.4324, 864.0))*5.3543)*2336.65)*0.02;
	gl_FragColor = vec4( c, 1.0 );
	
    
}
