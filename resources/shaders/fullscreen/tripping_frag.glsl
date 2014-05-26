uniform vec2 iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform vec2      iMouse;

// http://www.fractalforums.com/new-theories-and-research/very-simple-formula-for-fractal-patterns/

const int iterations=23;

void main(void)
{
	vec2 p=gl_FragCoord.xy / iResolution.xy-.5;
	p.y*=iResolution.y/iResolution.x;
	p*=.6+pow(sin(iGlobalTime*.15),4.)*10.;
	p+=vec2(sin(iGlobalTime*.2),cos(iGlobalTime*.1));
	p=vec2(p.x*cos(iGlobalTime*.2)-p.y*sin(iGlobalTime*.2),
		   p.y*cos(iGlobalTime*.2)+p.x*sin(iGlobalTime*.2));
	vec2 j=(1.-iMouse.xy/iResolution.xy)*1.3+vec2(.1);
	float expsmooth=0.;
	float average=0.;
	float l=length(p);
	float prevl;
	for (int i=0; i<iterations; i++) {
		p=abs(p*(1.5+cos(iGlobalTime*.1)*.5))/dot(p,p)-j;
		prevl=l;
		l=length(p);
		expsmooth+=exp(-1.2/abs(l-prevl));
		average+=abs(l-prevl);
	}
	float brightness=expsmooth*.1;
	average/=float(iterations)*15.;
	average+=iGlobalTime*.2;
	vec3 c=vec3(1.,.4,.3);
	vec3 color;
	color.r=mod(average,c.r*2.)-c.r;
	color.g=mod(average,c.g*2.)-c.g;
	color.b=mod(average,c.b*2.)-c.b;
	color=normalize(abs(color)+vec3(.3));
	gl_FragColor = vec4(color*brightness,1.0);
}
