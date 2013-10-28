uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float     iCellSize;
uniform float     iHighlight;
uniform float     iTimeStep1;
uniform float     iTimeStep2;
uniform float     iTimeStep3;
uniform float     iTimeStep4;
uniform float     iTimeStep5;
uniform float     iTimeStep6;
uniform float     iTimeStep7;
uniform vec4      iColor1;
//uniform vec4      iColor2;

// based on https://www.shadertoy.com/view/lsX3z4
float length2(vec2 p) { return dot(p, p); }

float noise(vec2 p){
	return fract(sin(fract(sin(p.x) * (4313.13311)) + p.y) * 3131.0011);
}

float worley(vec2 p) {
	float d = 1e30;
	for (int xo = -1; xo <= 1; ++xo)
        for (int yo = -1; yo <= 1; ++yo) {
            vec2 tp = floor(p) + vec2(xo, yo);
            d = min(d, length2(p - tp - vec2(noise(tp))));
        }
	return 3.*iHighlight*exp(-4.*abs(2.*d - 1.));
}

float fworley(vec2 p) {
	return sqrt(sqrt(sqrt(
                          pow(worley(p + iGlobalTime*iTimeStep1), 2.) *
                          worley(p*2. + 1.3 + iGlobalTime*iTimeStep2) *
                          worley(p*4. + 2.3 + iGlobalTime*iTimeStep3) *
                          worley(p*8. + 3.3 + iGlobalTime*iTimeStep4) *
                          worley(p*32. + 4.3 + iGlobalTime*iTimeStep5) *
                          sqrt(worley(p * 64. + 5.3 + iGlobalTime *iTimeStep6)) *
                          sqrt(sqrt(worley(p * 128. + 7.3 + iGlobalTime*iTimeStep7))))));
}

void main() {
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float t = fworley(uv * iResolution.xy / (iCellSize*1000.));
	t *= exp(-length2(abs(2.*uv - 1.)));
	float r = length(abs(2.*uv - 1.) * iResolution.xy);
	//gl_FragColor = vec4(t * vec3(1.8, 1.8*t, .1 + pow(t, 2.-t)), 1.);
    gl_FragColor = (3. * t) * iColor1;
}