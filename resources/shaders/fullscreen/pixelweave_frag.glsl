#version 120
uniform vec2 iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec2      iMouse;

// https://www.shadertoy.com/view/4dB3RK
// radius of the blur
const int bokehRad = 1;

// tweak this to get bigger bokeh by dithering
// this should be an integer
const float tapSpacing = 1.0;


vec3 hsv(float h, float s, float v)
{
    return pow(mix(vec3(1.0),clamp((abs(fract(h+vec3(3.0, 2.0, 1.0)/3.0)*6.0-3.0)-1.0), 0.0, 1.0),s)*v,vec3(2.2));
}

float shape(vec2 p)
{
    return abs(p.x)+abs(p.y)-1.0;
}

vec3 weave( vec2 pos )
{
	float a = .777+iGlobalTime*.0001*(1.0+.3*pow(length(pos.xy/iResolution.y),2.0));
	pos = pos*cos(a)+vec2(pos.y,-pos.x)*sin(a);
    pos = mod(pos/87.0, 2.0)-1.0;
    float h= abs(sin(0.3*iGlobalTime*shape(3.0*pos)));
    float c= 0.05/h;
    vec3 col = hsv(fract(0.1*iGlobalTime+h),1.0,1.0);
	return col*c;
}


void main()
{
	vec2 pos = gl_FragCoord.xy-iResolution.xy*.5-iMouse.xy;
	
	// I've written this so the compiler can, hopefully unroll all the integer maths to consts.
	vec3 col = vec3(0);
	int count = 0;
	const int h = ((bokehRad+1)*15)/13; // compiler won't let me cast to floats and do *2/sqrt(3) in a const
	for ( int i=-bokehRad; i <= bokehRad; i++ )
	{
		int ai = (i>0)?i:-i; // seriously? no int abs?
		for ( int j=-h; j <= h; j++ )
		{
			int aj = (j>0)?j:-j;
			if ( (h-aj)*2 > ai )
			{
				col += weave(pos+tapSpacing*vec2(i,j));
				count++;
			}
		}
	}
	
	col /= float(count);
    
    
	gl_FragColor = vec4(pow(col,vec3(1.0/2.2)),1.0);
}
