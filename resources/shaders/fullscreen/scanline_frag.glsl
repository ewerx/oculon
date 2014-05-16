#version 120
uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec2      iMouse;

// https://www.shadertoy.com/view/ldfGR2
// aji's amazing scanline shader

const float linecount = 60.0;
const vec4 gradA = vec4(0.0, 0.1, 0.0, 1.0);
const vec4 gradB = vec4(0.2, 0.5, 0.1, 1.0);
const vec4 gradC = vec4(0.9, 1.0, 0.6, 1.0);

vec2 pos, uv;

float noise(float factor)
{
	vec4 v = texture2D(iChannel1, uv + iGlobalTime * vec2(9.0, 7.0));
	return factor * v.x + (1.0 - factor);
}

vec4 base(void)
{
	return texture2D(iChannel0, uv + .1 * noise(1.0) * vec2(0.1, 0.0));
}

float triangle(float phase)
{
	phase *= 2.0;
	return 1.0 - abs(mod(phase, 2.0) - 1.0);
}

float scanline(float factor, float contrast)
{
	vec4 v = base();
	float lum = .2 * v.x + .5 * v.y + .3  * v.z;
	lum *= noise(0.3);
	float tri = triangle(pos.y * linecount);
	tri = pow(tri, contrast * (1.0 - lum) + .5);
	return tri * lum;
}

vec4 gradient(float i)
{
	i = clamp(i, 0.0, 1.0) * 2.0;
	if (i < 1.0) {
		return (1.0 - i) * gradA + i * gradB;
	} else {
		i -= 1.0;
		return (1.0 - i) * gradB + i * gradC;
	}
}

vec4 vignette(vec4 at)
{
	float dx = 1.3 * abs(pos.x - .5);
	float dy = 1.3 * abs(pos.y - .5);
    return at * (1.0 - dx * dx - dy * dy);
}

void main(void)
{
	pos = uv = (gl_FragCoord.xy - vec2(0.0, 0.5)) / iResolution.xy;
	uv.y = floor(uv.y * linecount) / linecount;
	gl_FragColor = vignette(gradient(scanline(0.8, 2.0)));
}
