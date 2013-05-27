uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)


uniform float   timeScale;
uniform vec3    colorScale;
uniform int     rPower;
uniform int     gPower;    
uniform int     bPower;
uniform float   strengthFactor;
uniform float   strengthMin;
uniform float   strengthConst;
uniform int     iterations;
uniform vec3    magnitude;
uniform float   accumPower;
uniform float   fieldScale;
uniform float   fieldSubtract;
uniform vec3    panSpeed;
uniform vec3    uvOffset;
uniform float   uvScale;

// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/
float field(in vec3 p) {
	float strength = strengthMin + strengthFactor * log(1.e-6 + fract(sin(iGlobalTime * timeScale) * strengthConst));
	float accum = 0.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < iterations; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + magnitude;//vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), accumPower));
		tw += w;
		prev = mag;
	}
	return max(0., fieldScale * accum / tw - fieldSubtract);
}

void main() {
	vec2 uv = 2. * gl_FragCoord.xy / iResolution.xy - 1.;
	vec2 uvs = uv * iResolution.xy / max(iResolution.x, iResolution.y);
	vec3 p = vec3(uvs * uvScale, 0) + uvOffset;//vec3(1., -1.3, 0.);
	p += .2 * vec3(sin(iGlobalTime * panSpeed.x), sin(iGlobalTime * panSpeed.y), sin(iGlobalTime * panSpeed.z));
	float t = field(p);
	float v = (1. - exp((abs(uv.x) - 1.) * 6.)) * (1. - exp((abs(uv.y) - 1.) * 6.));
	//gl_FragColor = mix(.4, 1., v) * vec4(1.8 * t * t * t, 1.4 * t * t, t, 1.0);
    
    float r = t * colorScale.x;
    for (int i = 0; i < rPower; ++i) {
        r *= t;
    }
    
    float g = t * colorScale.y;
    for (int i = 0; i < gPower; ++i) {
        g *= t;
    }
    
    float b = t * colorScale.z;
    for (int i = 0; i < bPower; ++i) {
        b *= t;
    }
    
    gl_FragColor = mix(.4, 1., v) * vec4(r, g, b, 1.0);

    //gl_FragColor = mix(.4, 1., v) * vec4(t * t, t * t, t * t, 1.0);
}