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
uniform float       iMorphSpeed;


// http://www.fractalforums.com/movies-showcase-%28rate-my-movie%29/very-rare-deep-sea-fractal-creature/

const int Iterations=25;
const float Scale=1.27;
const vec3 Julia=vec3(-2.,-1.5,-.5);
const vec3 RotVector=vec3(0.5,-0.05,-0.5);
const float RotAngle=40.;
const float Speed=1.3;
const float Amplitude=0.45;
const float detail=.025;
const vec3 lightdir=-vec3(0.5,1.,0.5);

mat2 rot;

float de(vec3 p);

vec3 normal(vec3 p) {
	vec3 e = vec3(0.0,detail,0.0);
	
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

float light(in vec3 p, in vec3 dir) {
	vec3 ldir=normalize(lightdir);
	vec3 n=normal(p);
	float sh=softshadow(p,-ldir,1.,20.);
	float diff=max(0.,dot(ldir,-n));
	vec3 r = reflect(ldir,n);
	float spec=max(0.,dot(dir,-r));
	return diff*sh+pow(spec,30.)*.5*sh+.15*max(0.,dot(normalize(dir),-n));
}

float raymarch(in vec3 from, in vec3 dir)
{
	float st,d=1.0,col,totdist=st=0.;
	vec3 p;
	for (int i=0; i<70; i++) {
        if (d>detail && totdist<50.)
        {
            p=from+totdist*dir;
            d=de(p);
            totdist+=d;
        }
	}
    // background color
	float backg=0.0;
	if (d<detail) {
		col=light(p-detail*dir, dir);
	} else {
		col=backg;
	}
	col = mix(col, backg, 1.0-exp(-.000025*pow(totdist,3.5)));
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
	float time=iGlobalTime*Speed;
	vec3 ani;
	ani=vec3(sin(time),sin(time),cos(time))*Amplitude;
	p+=sin(p*3.+time*6.)*.04;
	mat3 rot = rotationMatrix3(normalize(RotVector+ani), RotAngle+sin(time)*10.);
	vec3 pp=p;
	float l;
	for (int i=0; i<Iterations; i++) {
		p.xy=abs(p.xy);
		p=p*Scale+Julia;
		p*=rot;
		l=length(p);
	}
	return l*pow(Scale, -float(Iterations))*.9;
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
    
	float col=raymarch(from,dir);
	gl_FragColor = vec4(col,col,col,1.0);
}
