//
//  livingkifs_frag.glsl
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

uniform vec3        iResolution;     // viewport resolution (in pixels)
uniform float       iGlobalTime;     // shader playback time (in seconds)
uniform float       iBackgroundAlpha;
uniform sampler2D   iColorMap;
uniform vec4        iColor1;

uniform int         iIterations;
uniform vec3        iJulia;
uniform vec3        iRotation;
uniform vec3        iLightDir;
uniform float       iScale;
uniform float       iRotAngle;
uniform float       iAmplitude;
uniform float       iDetail;

// http://www.fractalforums.com/movies-showcase-%28rate-my-movie%29/very-rare-deep-sea-fractal-creature/


mat2 rot;

float de(vec3 p);

vec3 normal(vec3 p) {
	vec3 e = vec3(0.0,iDetail,0.0);
	
	return normalize(vec3(
                          de(p+e.yxx)-de(p-e.yxx),
                          de(p+e.xyx)-de(p-e.xyx),
                          de(p+e.xxy)-de(p-e.xxy)
                          )
                     );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = de(ro + rd*t);
		h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.01, 0.5 );
    }
    return clamp(res,0.0,1.0);
}

vec3 light(in vec3 p, in vec3 dir, float d) {
	vec3 ldir=normalize(iLightDir);
	vec3 n=normal(p);
	float sh=softshadow(p,-ldir,1.,20.);
	float diff=max(0.,dot(ldir,-n));
	vec3 r = reflect(ldir,n);
	float spec=max(0.,dot(dir,-r));
    // uncomment for color map
    //vec3 ray = .8*d*((0.4*p-3.*r)+d*vec3(1.0,1.0,1.0));
	//vec4 lightCol = texture2D(iColorMap,ray.xz+ray.xy);
    //return 3.0*lightCol*diff*sh+pow(spec,30.)*.5*sh+.15*max(0.,dot(normalize(dir),-n));
	return iColor1.xyz*diff*sh+pow(spec,30.)*.5*sh+.15*max(0.,dot(normalize(dir),-n));
}

vec4 raymarch(in vec3 from, in vec3 dir)
{
	float st,d=1.0,totdist=st=0.;
	vec3 p;
    vec4 col;
	for (int i=0; i<70; i++) {
        if (d>iDetail && totdist<50.)
        {
            p=from+totdist*dir;
            d=de(p);
            totdist+=d;
        }
	}
    // background color
	vec4 backg=vec4(0.0,0.0,0.0,iBackgroundAlpha);
	if (d<iDetail) {
		//col=light(p-iDetail*dir, dir, d);
        col=vec4(light(p, dir, d),1.0);
	} else {
		col=backg;
	}
	//col = mix(col, backg, 1.0-exp(-.000025*pow(totdist,3.5)));
	return col;
}

mat3  rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
                (1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
                (1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
                );
}

float de(vec3 p) {
	p=p.zxy;
	float a=1.5+sin(iGlobalTime*.5)*.5;
	p.xy=p.xy*mat2(cos(a),sin(a),-sin(a),cos(a));
	p.x*=.75;
	float time=iGlobalTime;
	vec3 ani;
	ani=vec3(sin(time),sin(time),cos(time))*iAmplitude;
	p+=sin(p*3.+time*6.)*.04;
	mat3 rot = rotationMatrix3(normalize(iRotation+ani), iRotAngle+sin(time)*10.);
	vec3 pp=p;
	float l;
	for (int i=0; i<iIterations; i++) {
		p.xy=abs(p.xy);
		p=p*iScale+iJulia;
		p*=rot;
		l=length(p);
	}
	return l*pow(iScale, -float(iIterations))*.9;
}

void main(void)
{
	float t=iGlobalTime*.3;
	vec2 uv = gl_FragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;
	vec3 from=vec3(0.,-.7,-20.);
	vec3 dir=normalize(vec3(uv*.7,1.));
	rot=mat2(cos(-.5),sin(-.5),-sin(-.5),cos(-.5));
	dir.yz=dir.yz*rot;
	from.yz=from.yz*rot;
    
	vec4 col=raymarch(from,dir);
	gl_FragColor = col;//vec4(col.r,col.g,col.b,iBackgroundAlpha);
}
