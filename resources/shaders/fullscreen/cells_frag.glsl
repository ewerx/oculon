uniform vec2 iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float     iZoom;
uniform float     iHighlight;
uniform float     iTimeStep1;
uniform float     iTimeStep2;
uniform float     iTimeStep3;
uniform float     iTimeStep4;
uniform float     iTimeStep5;
uniform float     iTimeStep6;
uniform float     iTimeStep7;
uniform float     iFrequency1;
uniform float     iFrequency2;
uniform float     iFrequency3;
uniform float     iFrequency4;
uniform float     iFrequency5;
uniform float     iFrequency6;
uniform float     iFrequency7;
uniform float     iBrightness1;
uniform float     iBrightness2;
uniform float     iBrightness3;
uniform float     iBrightness4;
uniform float     iBrightness5;
uniform float     iBrightness6;
uniform float     iBrightness7;
uniform float     iIntensity;
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
	return 3.*exp(-4.*abs(2.*d - 1.));
}

float fworley(vec2 p) {
	return sqrt(sqrt(sqrt(
                          iBrightness1*pow(worley(p*iFrequency1 + iTimeStep1), iIntensity) *
                          iBrightness2*worley(p*iFrequency2 + 1.3 + iTimeStep2) *
                          iBrightness3*worley(p*iFrequency3 + 2.3 + iTimeStep3) *
                          iBrightness4*worley(p*iFrequency4 + 3.3 + iTimeStep4) *
                          iBrightness5*worley(p*iFrequency5 + 4.3 + iTimeStep5) *
                          sqrt(iBrightness6*worley(p * iFrequency6 + 5.3 + iTimeStep6)) *
                          sqrt(sqrt(iBrightness7*worley(p * iFrequency7 + 7.3 + iTimeStep7))))));
}

void main() {
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float t = fworley(uv * iResolution.xy / (iZoom*1000.));
	t *= exp(-length2(abs(2.*uv - 1.)));
	float r = length(abs(2.*uv - 1.) * iResolution.xy);
	//gl_FragColor = vec4(t * vec3(1.8, 1.8*t, .1 + pow(t, 2.-t)), 1.);
    gl_FragColor = (3. * t) * iColor1;
}