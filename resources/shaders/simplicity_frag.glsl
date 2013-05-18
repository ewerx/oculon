uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)


uniform vec3      timeScale;
uniform vec3      colorScale;
uniform int     rPower;
uniform int     gPower;    
uniform int     bPower;

// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/
float field(in vec3 p) {
	float strength = 7. + .03 * log(1.e-6 + fract(sin(iGlobalTime) * 4373.11));
	float accum = 0.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 32; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), 2.3));
		tw += w;
		prev = mag;
	}
	return max(0., 5. * accum / tw - .7);
}

void main() {
	vec2 uv = 2. * gl_FragCoord.xy / iResolution.xy - 1.;
	vec2 uvs = uv * iResolution.xy / max(iResolution.x, iResolution.y);
	vec3 p = vec3(uvs / 4., 0) + vec3(1., -1.3, 0.);
	p += .2 * vec3(sin(iGlobalTime / 16.), sin(iGlobalTime / 12.),  sin(iGlobalTime / 128.));
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