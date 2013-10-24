uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform vec2      iMouse;

#define TIME iGlobalTime

mat2 rotate(float a) { return mat2(cos(a),-sin(a),sin(a),cos(a)); }

float noise(float x) { return texture2D(iChannel0,vec2(x)).x;}

#define WEBN 64.

void main(void) {
	vec2 p = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	float l = 0.0;
	float mz = 0.0;
    
	for (float i=0.; i < WEBN; i+=1.0) {
		float fi = 0.0;//i/WEBN*acos(-1.)*0.001;
		float n = noise(fi);
		
		float z = 1.0-mod(TIME/7.0+i/WEBN,1.);
		vec2 o = p;
		//o -= vec2(sin(2.5*fi+TIME)*2.0,cos(2.0*fi+TIME))*z;
		o *= rotate(TIME*n*1.0)/(z*1.);
		l += (smoothstep(0.98,1.0,sin(mod(length(o),0.5)*30.0)))*(0.5-z);
		mz = min(mz,z);
	}
	
	vec3 col = mix(vec3(0.2,0.0,0.1),vec3(0.9,1.0,1.0),l*(0.55+mz));
	gl_FragColor = vec4(col,1.0);
}
