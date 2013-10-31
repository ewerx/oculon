uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform float        iRings;
uniform float       iTimeScale;
uniform float       iSmoothing;
uniform vec4        iColor1;
uniform vec4        iColor2;
uniform float       iIntervals;

// based on https://www.shadertoy.com/view/XsXGD8

//const float velocity=-1.;
//const float b = 0.003;		//size of the smoothed border

void main()
{
    float b = iSmoothing;
	vec2 position = gl_FragCoord.xy/iResolution.xy;
    float aspect = iResolution.x/iResolution.y;
	position.x *= aspect;
	float dist = distance(position, vec2(aspect*0.5, 0.5));
	float offset=iGlobalTime*iTimeScale;
	float conv=iRings;
	float v=dist*conv-offset;
	float ringr=floor(v);
	float color=smoothstep(-b, b, abs(dist- (ringr+float(fract(v)>0.5)+offset)/conv));
	if(mod(ringr,iIntervals)==1.)
		color=1.-color;
	gl_FragColor = color * iColor1;//vec4(color, color, color, 1.);
}

