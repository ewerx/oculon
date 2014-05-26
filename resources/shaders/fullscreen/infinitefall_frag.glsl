//
//  livingkifs_frag.glsl
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

uniform vec2 iResolution;     // viewport resolution (in pixels)
uniform float       iGlobalTime;     // shader playback time (in seconds)
uniform float       iBackgroundAlpha;
uniform float       iMorphSpeed;
uniform sampler2D   iChannel0;


//by mu6k
//License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
//muuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuusk!

float hash(float x)
{
	return fract(sin(cos(x*12.13)*19.123)*17.321);
}


float noise(vec2 p)
{
	vec2 pm = mod(p,1.0);
	vec2 pd = p-pm;
	float v0=hash(pd.x+pd.y*41.0);
	float v1=hash(pd.x+1.0+pd.y*41.0);
	float v2=hash(pd.x+pd.y*41.0+41.0);
	float v3=hash(pd.x+pd.y*41.0+42.0);
	v0 = mix(v0,v1,smoothstep(0.0,1.0,pm.x));
	v2 = mix(v2,v3,smoothstep(0.0,1.0,pm.x));
	return mix(v0,v2,smoothstep(0.0,1.0,pm.y));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy-0.5;
	uv.x*=iResolution.x/iResolution.y;
	float v =0.0;
	
	vec2 tuv = uv;
	
	float rot=sin(iGlobalTime*0.3)*sin(iGlobalTime*0.4)*0.2;
    
	uv.x = tuv.x*cos(rot)-tuv.y*sin(rot);
	uv.y = tuv.x*sin(rot)+tuv.y*cos(rot);
	
	for (float i = 0.0; i<12.0; i+=1.0)
	{
		float t = mod(iGlobalTime+i,12.0);
		float l = iGlobalTime-t;
		float e = pow(2.0,t);
		v+=noise(uv*e+vec2(cos(l)*53.0,sin(l)*100.0))*(1.0-(t/12.0))*(t/12.0);
		
	}
	
	v-=0.5;
	v*=1.0;
	
	vec3 color = vec3(v,v,v);
    
	gl_FragColor = vec4(color,1.0);
}


/*
//--- infinite fall --- Fabrice NEYRET  august 2013

#define SPEED 1.0
#define SHAKE 0.
#define ROTATE 0.1

#define Pi 3.1415927

// --- base noise
float tex(vec2 uv, float va)
{
	float n = texture2D(iChannel0,uv).r;
	//n = .5+.5*cos(2.*Pi*(n-.1*va*t));
	//float n2 = texture2D(iChannel0,uv+1./512.).r;
	//n=.5+.5*cos(va*t+atan(2.*n2-1.,2.*n-1.));
	//float n2 = texture2D(iChannel0,uv+.5).r;
	//n = mix(n,n2,.5+.5*cos(va*t));
	
#define MODE 0  // kind of noise texture
#if MODE==0
#define A 2.
	return n;
#elif MODE==1
#define A 3.
	return 2.*n-1.;
#elif MODE==2
#define A 3.
	return abs(2.*n-1.);
#elif MODE==3
#define A 1.5
	return 1.-abs(2.*n-1.);
#endif
}


// --- infinite perlin noise
float noise(vec2 uv, float z)
{
	float v=0.,p=0.;
	float co=cos(1.7),si=sin(1.7); mat2 M = mat2(co,-si,si,co);
	const int L = 20;
	for (int i=0; i<L; i++)
	{
		float k = float(i)-z;
		float a =  (.5-.5*cos(2.*Pi*k/float(L)));
		float s = pow(2., fract(k/float(L))*float(L));
		v += a/s* tex(.001*(M*uv)*s,k); M *=M;
		p += a/s;
	}
	
    return A*v/p;
}

void main(void)
{
	float t = SPEED*iGlobalTime;
	vec2 uv = gl_FragCoord.xy / iResolution.y-vec2(.8,.5);
    
	vec2 mouse=vec2(0.);
	//if (iMouse.x>0.) mouse = iMouse.xy/ iResolution.y-vec2(.8,.5);
	
	uv.x += SHAKE/100.*pow(texture2D(iChannel0,vec2(t,.5)).r,4.);
    
	float va = ROTATE; // mouse.x;
	float co=cos(va*t),si=sin(va*t);uv = mat2(co,-si,si,co)*uv;
    
	// uv *= pow(8.,mouse.y);
	uv -= 4.*mouse;
	
    
    // terrain and normals
	float v,vx,vy;
	vec2 eps = vec2(1./256.,0.);
	v = noise(uv,t);
	vec2 N = (vec2(noise(uv+eps.xy,t), noise(uv+eps.yx,t))-v)*256.;
    
	
	// shading
	
	vec2 LUM = vec2(.2,.8);
	float lum = clamp(max(.1,dot(N,LUM)),0.,1.);
    
	//v = smoothstep(.2,.9,v/2.);
	v = pow(v,3.);
	//v *= 2.;
	vec3 col = vec3(v,v/2.,v/4.)*lum;
	
	gl_FragColor = vec4(col,1.);
}
*/