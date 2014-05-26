uniform vec2 iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec2      iMouse;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform vec4      iColor3;
uniform int       iIterations;
uniform float     iScale;
uniform float     iSaturation;
uniform float     iBrightness;
uniform float     iContrast;
uniform float     iMinBrightness;
uniform vec2      iJulia;
uniform vec3      iOrbitTraps;
uniform vec3      iTrapWidths;
uniform vec3      iTrapBrightness;
uniform vec3      iTrapContrast;

// https://www.shadertoy.com/view/ld2GRG
//orbit traps from julia version of fractal formula z=(z+1/z+c)*-scale;

#define antialias 1.

void main(void)
{
	vec3 aacolor=vec3(0.);
	vec2 uv=gl_FragCoord.xy / iResolution.xy - 0.5;
	uv.x*=iResolution.x/iResolution.y;
	vec2 pos=uv.xy;
	float t=iGlobalTime*.02;
	float zoo=.07-sin(t*2.)*.04;
	pos*=zoo;
	float a=radians(-50.+sin(t*8.)*30.);
	pos*=mat2(cos(a),sin(a),-sin(a),cos(a));
	pos+=vec2(.75,.1)+vec2(-sin(t*2.)*2.,-cos(t)*.2)*.13;
	vec2 pixsize=2./iResolution.xy*zoo;
	float av=0.;
	vec3 its=vec3(0.);
	for (float aa=0.; aa<9.; aa++) {
		vec3 otrap=vec3(1000.);
		if (aa<antialias*antialias) {
			vec2 aacoord=floor(vec2(aa/antialias,mod(aa,antialias)));
			vec2 z=pos+aacoord*pixsize/antialias;
			for (int i=0; i<iIterations; i++) {
				vec2 cz=vec2(z.x,-z.y);
				z=z+cz/dot(z,z)+iJulia;
				z=z*iScale;
				float l=length(z);
				vec3 ot=abs(vec3(l)-iOrbitTraps);
				if (ot.x<otrap.x) {
					otrap.x=ot.x;
					its.x=float(iIterations-i);
				}
				if (ot.y<otrap.y) {
					otrap.y=ot.y;
					its.y=float(iIterations-i);
				}
				if (ot.z<otrap.z) {
					otrap.z=ot.z;
					its.z=float(iIterations-i);
				}
			}
		}
		otrap=pow(max(vec3(0.0),iTrapWidths-otrap)/iTrapWidths,iTrapContrast);
		its=pow(its/float(iIterations),vec3(.2));
		vec3 otcol1=otrap.x*pow(iColor1.rgb,3.5-vec3(its.x*3.))*max(iMinBrightness,its.x)*iTrapBrightness.x;
		vec3 otcol2=otrap.y*pow(iColor2.rgb,3.5-vec3(its.y*3.))*max(iMinBrightness,its.y)*iTrapBrightness.y;
		vec3 otcol3=otrap.z*pow(iColor3.rgb,3.5-vec3(its.z*3.))*max(iMinBrightness,its.z)*iTrapBrightness.z;
		aacolor+=(otcol1+otcol2+otcol3)/3.;
	}
	aacolor=aacolor/(antialias*antialias);
	vec3 color=mix(vec3(length(aacolor)),aacolor,iSaturation)*iBrightness;
	color=pow(abs(color),vec3(iContrast))+vec3(.05,.05,.08);
	gl_FragColor = vec4(color,1.0);
}