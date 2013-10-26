uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform int       pattern;

// based on https://www.shadertoy.com/view/Msl3Dj
float pattern0(vec2 p, float time) { return (sin((abs(p.x)+abs(p.y))*50.0+time*10.0)+1.0)/2.0; }
float pattern1(vec2 p, float time) { return (sin(length(p)*50.0+abs(sin(atan(p.y,p.x)*10.0+time*4.0)*length(p)*5.0)+time*10.0)+1.0)/2.0; }
float pattern2(vec2 p, float time) { return sin(atan(p.y,p.x)*20.0+time*20.0); }

int getPosition(vec2 p) {
	if (p.y < -0.8 && abs(p.x) < 0.3) {
		if (p.x < -0.1) return 0;
		else if (p.x > 0.1) return 2;
	}
	return 1;
}

void main(void) {
	vec2 p = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	int mp = 0;//TODO: user pattern index as input
	
	float p0,p1;
	
	if (mp == 0) {
		p0 = pattern0(p,-iGlobalTime);
		p1 = pattern0(p, iGlobalTime);
	} else if (mp == 1) {
		p0 = pattern1(p,-iGlobalTime);
		p1 = pattern1(p, iGlobalTime);
	} else {
		p0 = pattern2(p,-iGlobalTime);
		p1 = pattern2(p, iGlobalTime);
	}
	
	float s = mix(p0,p1,smoothstep(0.5,0.55,length(p)));
	gl_FragColor = vec4(vec3(s),1.0);
}