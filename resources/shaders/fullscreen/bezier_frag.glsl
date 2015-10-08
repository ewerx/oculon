#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform int       iSamples;
uniform float     iThickness;
uniform float     iBlur;
uniform float     iFrequency;
uniform float     iPoint1Range;
uniform float     iPoint2Range;

#define time iGlobalTime
#define resolution iResolution
#define mouse iMouse

// https://www.shadertoy.com/view/4dfSW4
#define PI			3.14159265359

vec2 fBezier2 (vec2 p0, vec2 p1, vec2 p2, float t)
{
	return mix (
                mix (p0, p1, t),
                mix (p1, p2, t),t);
}

vec2 fBezier3 (vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t)
{
	return fBezier2 (
                     mix (p0, p1, t),
                     mix (p1, p2, t),
                     mix (p2, p3, t),t);
}


float fGetDistance (vec2 p, vec2 p0, vec2 p1, vec2 p2, vec2 p3)
{
	float d0 = distance (p, p0) ;
	for (int i = 0;i < iSamples;++i) {
		float d = distance (p, fBezier3 (p0, p1, p2, p3, float (i)/float (iSamples)));
		d0 = min (d0, d);
	}
	return d0;
}

void main (void)
{
	float t = iGlobalTime;
	vec2 uv = (2.0 * gl_FragCoord.xy - iResolution.xy)/ min (iResolution.x, iResolution.y);
	
	vec2 p1 = vec2 (-0.5, 0.0);
	vec2 p2 = vec2 (0.5, 0.0);
	vec2 p0 = vec2 (sin (t), cos (t))*iPoint1Range + p1;
	vec2 p3 = vec2 (cos (t), sin (t))*iPoint2Range + p2;
	
	float d = fGetDistance (uv, p0, p1, p2, p3);
	float s = smoothstep (iThickness*(1.0 - iBlur), iThickness*(1.0 + iBlur), d);
	
	gl_FragColor = mix (iColor2, iColor1, 0.5 + 0.5*sin (s*PI*iFrequency) );
}
