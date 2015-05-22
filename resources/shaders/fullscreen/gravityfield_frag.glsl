#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform int       iMode;
uniform int       iPoints;
uniform float     iPhase;
uniform float     iField;
uniform float     iSpan;
uniform float     iBrightness;

#define POINTS iPoints 		 // number of stars

// --- GUI utils

float t = iGlobalTime;

//bool keyToggle(int ascii) {
//	return (texture2D(iChannel2,vec2((.5+float(ascii))/256.,0.75)).x > 0.);
//}


// --- math utils

float dist2(vec2 P0, vec2 P1) { vec2 D=P1-P0; return dot(D,D); }

float hash (float i) { return 2.*fract(sin(i*7467.25)*1e5) - 1.; }
vec2  hash2(float i) { return vec2(hash(i),hash(i-.1)); }
vec4  hash4(float i) { return vec4(hash(i),hash(i-.1),hash(i-.3),hash(i+.1)); }



// === main ===================

// motion of stars
vec2 P(float i) {
	vec4 c = hash4(i);
	return vec2(  cos(t*c.x-c.z)+.5*cos(2.765*t*c.y+c.w),
                ( sin(t*c.y-c.w)+.5*sin(1.893*t*c.x+c.z) )*iSpan	 );
}

// ---

void main(void)
{
	vec2 uv    = 2.*((gl_FragCoord.xy / iResolution.y) - vec2(.8,.5));
    float my = .5*(pow(iField,3.));
	int MODE = iMode;
    float fMODE = iPhase;
    
	const int R = 1;
	
	float v=0.;
    vec2 V=vec2(0.);
    
	for (int i=1; i<POINTS; i++) { // sums stars
		vec2 p = P(float(i));
		for (int y=-R; y<=R; y++)  // ghost echos in cycling universe
			for (int x=-R; x<=R; x++) {
				vec2 d = p+2.*vec2(float(x),float(y)) -uv; // pixel to star
				float r2 = dot(d,d);
				r2 = clamp(r2,5e-2*my,1e3);
				V +=  d / r2;  // gravity force field
			}
    }
	
	v = length(V);
	v *= 1./(9.*float(POINTS));
    ///v = clamp(v,0.,.1);
	
	v *= 2.+100.*fMODE;
	if (MODE==0) gl_FragColor = vec4(.2*v)+smoothstep(.05,.0,abs(v-5.*my))*vec4(sin(v));
	if (MODE==1) gl_FragColor = vec4(0.5 + 0.5*sin(2.*v));
	if (MODE==2) gl_FragColor = vec4(sin(v),sin(v/2.),sin(v/4.),1.);
    
    
}
