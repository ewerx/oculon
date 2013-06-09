uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;

uniform float   timeScale;
uniform vec3    colorScale;
uniform int     rPower;
uniform int     gPower;    
uniform int     bPower;

//parameters
uniform int iterations;

uniform float scale;

uniform vec2 fold;

uniform vec2 translate;

uniform float zoom;

uniform float brightness;

uniform float saturation;

uniform float texturescale;

uniform float rotspeed;

uniform float colspeed;

uniform float antialias;


vec2 rotate(vec2 p, float angle) {
return vec2(p.x*cos(angle)-p.y*sin(angle),
		   p.y*cos(angle)+p.x*sin(angle));
}

void main(void)
{
	vec3 aacolor=vec3(0.);
	vec2 pos=gl_FragCoord.xy / iResolution.xy-.5;
	float aspect=iResolution.y/iResolution.x;
	pos.y*=aspect;
	pos/=zoom; 
	vec2 pixsize=max(1./zoom,100.-iGlobalTime*50.)/iResolution.xy;
	pixsize.y*=aspect;
	for (float aa=0.; aa<25.; aa++) {
		if (aa+1.>antialias*antialias) break;
		vec2 aacoord=floor(vec2(aa/antialias,mod(aa,antialias)));
		vec2 p=pos+aacoord*pixsize/antialias;
		p+=fold;
		float expsmooth=0.;
		vec2 average=vec2(0.);
		float l=length(p);
		for (int i=0; i<iterations; i++) {
			p=abs(p-fold)+fold;
			p=p*scale-translate;
			if (length(p)>20.) break;
			p=rotate(p,iGlobalTime*rotspeed);
			average+=p;
		}
		average/=float(iterations);
		vec2 coord=average+vec2(iGlobalTime*colspeed);
		vec3 color=texture2D(iChannel0,coord*texturescale).xyz;
		color*=min(1.1,length(average)*brightness);
		color=mix(vec3(length(color)),color,saturation);
		aacolor+=color;
	}
	gl_FragColor = vec4(aacolor/(antialias*antialias),1.0);
}