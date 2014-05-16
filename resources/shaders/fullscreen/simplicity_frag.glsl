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
uniform vec3    panPos;
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
	p += .2 * vec3(sin(panPos.x), sin(panPos.y), sin(panPos.z));
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

// Simplicity Galaxy (audio reactive)

//CBS
//Parallax scrolling fractal galaxy.
//Inspired by JoshP's Simplicity shader: https://www.shadertoy.com/view/lslGWr

// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/
/*
float field(in vec3 p,float s) {
	float strength = 7. + .03 * log(1.e-6 + fract(sin(iGlobalTime) * 4373.11));
	float accum = s/4.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 26; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), 2.2));
		tw += w;
		prev = mag;
	}
	return max(0., 5. * accum / tw - .7);
}

// Less iterations for second layer
float field2(in vec3 p, float s) {
	float strength = 7. + .03 * log(1.e-6 + fract(sin(iGlobalTime) * 4373.11));
	float accum = s/4.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 18; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), 2.2));
		tw += w;
		prev = mag;
	}
	return max(0., 5. * accum / tw - .7);
}

vec3 nrand3( vec2 co )
{
	vec3 a = fract( cos( co.x*8.3e-3 + co.y )*vec3(1.3e5, 4.7e5, 2.9e5) );
	vec3 b = fract( sin( co.x*0.3e-3 + co.y )*vec3(8.1e5, 1.0e5, 0.1e5) );
	vec3 c = mix(a, b, 0.5);
	return c;
}


void main() {
    vec2 uv = 2. * gl_FragCoord.xy / iResolution.xy - 1.;
	vec2 uvs = uv * iResolution.xy / max(iResolution.x, iResolution.y);
	vec3 p = vec3(uvs / 4., 0) + vec3(1., -1.3, 0.);
	p += .2 * vec3(sin(iGlobalTime / 16.), sin(iGlobalTime / 12.),  sin(iGlobalTime / 128.));
	
	float freqs[4];
	//Sound
	freqs[0] = texture2D( iChannel0, vec2( 0.01, 0.25 ) ).x;
	freqs[1] = texture2D( iChannel0, vec2( 0.07, 0.25 ) ).x;
	freqs[2] = texture2D( iChannel0, vec2( 0.15, 0.25 ) ).x;
	freqs[3] = texture2D( iChannel0, vec2( 0.30, 0.25 ) ).x;
    
	float t = field(p,freqs[2]);
	float v = (1. - exp((abs(uv.x) - 1.) * 6.)) * (1. - exp((abs(uv.y) - 1.) * 6.));
	
    //Second Layer
	vec3 p2 = vec3(uvs / (4.+sin(iGlobalTime*0.11)*0.2+0.2+sin(iGlobalTime*0.15)*0.3+0.4), 1.5) + vec3(2., -1.3, -1.);
	p2 += 0.25 * vec3(sin(iGlobalTime / 16.), sin(iGlobalTime / 12.),  sin(iGlobalTime / 128.));
	float t2 = field2(p2,freqs[3]);
	vec4 c2 = mix(.4, 1., v) * vec4(1.3 * t2 * t2 * t2 ,1.8  * t2 * t2 , t2* freqs[0], t2);
	
	
	//Let's add some stars
	//Thanks to http://glsl.heroku.com/e#6904.0
	vec2 seed = p.xy * 2.0;
	seed = floor(seed * iResolution.x);
	vec3 rnd = nrand3( seed );
	vec4 starcolor = vec4(pow(rnd.y,40.0));
	
	//Second Layer
	vec2 seed2 = p2.xy * 2.0;
	seed2 = floor(seed2 * iResolution.x);
	vec3 rnd2 = nrand3( seed2 );
	starcolor += vec4(pow(rnd2.y,40.0));
	
	gl_FragColor = mix(freqs[3]-.3, 1., v) * vec4(1.5*freqs[2] * t * t* t , 1.2*freqs[1] * t * t, freqs[3]*t, 1.0)+c2+starcolor;
}
*/