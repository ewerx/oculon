#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec2      iMouse;

//Triangulator by nimitz (twitter: @stormoid)

#define ITR 40
#define FAR 100.
#define time iGlobalTime*0.2

mat2 mm2(in float a){float c = cos(a), s = sin(a);return mat2(c,-s,s,c);}
mat2 m2 = mat2(0.934, 0.358, -0.358, 0.934);
float tri(in float x){return abs(fract(x)-0.5);}

float heightmap(in vec2 p)
{
    p*=.05;
    float z=2.;
    float rz = 0.;
    for (float i= 1.;i < 4.;i++ )
    {
        rz+= tri(p.x+tri(p.y*1.5))/z;
        z = z*-.85;
        p = p*1.32;
        p*= m2;
    }
    rz += sin(p.y+sin(p.x*.9))*.7+.3;
    return rz*5.;
}

//from jessifin (https://www.shadertoy.com/view/lslXDf)
vec3 bary(vec2 a, vec2 b, vec2 c, vec2 p)
{
    vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float inv_denom = 1.0 / (v0.x * v1.y - v1.x * v0.y)+1e-9;
    float v = (v2.x * v1.y - v1.x * v2.y) * inv_denom;
    float w = (v0.x * v2.y - v2.x * v0.y) * inv_denom;
    float u = 1.0 - v - w;
    return abs(vec3(u,v,w));
}

/*
	Idea is quite simple, find which (triangular) side of a given tile we're in,
	then get 3 samples and compute height using barycentric coordinates.
 */
float map(vec3 p)
{
    vec3 q = fract(p)-0.5;
    vec3 iq = floor(p);
    vec2 p1 = vec2(iq.x-.5, iq.z+.5);
    vec2 p2 = vec2(iq.x+.5, iq.z-.5);
    
    float d1 = heightmap(p1);
    float d2 = heightmap(p2);
    
    float sw = sign(q.x+q.z);
    vec2 px = vec2(iq.x+.5*sw, iq.z+.5*sw);
    float dx = heightmap(px);
    vec3 bar = bary(vec2(.5*sw,.5*sw),vec2(-.5,.5),vec2(.5,-.5), q.xz);
    return (bar.x*dx + bar.y*d1 + bar.z*d2 + p.y + 3.)*.9;
}

float march(in vec3 ro, in vec3 rd)
{
    float precis = 0.001;
    float h=precis*2.0;
    float d = 0.;
    for( int i=0; i<ITR; i++ )
    {
        if( abs(h)<precis || d>FAR ) break;
        d += h;
        float res = map(ro+rd*d)*1.1;
        h = res;
    }
    return d;
}

vec3 normal(const in vec3 p)
{
    vec2 e = vec2(-1., 1.)*0.01;
    return normalize(e.yxx*map(p + e.yxx) + e.xxy*map(p + e.xxy) +
                     e.xyx*map(p + e.xyx) + e.yyy*map(p + e.yyy) );
}

void main(void)
{
    vec2 bp = gl_FragCoord.xy/iResolution.xy;
    vec2 p = bp-0.5;
    p.x*=iResolution.x/iResolution.y;
    vec2 um = vec2(0.45+sin(time*0.7)*2., -.18);
    
    vec3 ro = vec3(sin(time*0.7+1.)*20.,3., time*50.);
    vec3 eye = normalize(vec3(cos(um.x), um.y*5., sin(um.x)));
    vec3 right = normalize(vec3(cos(um.x+1.5708), 0., sin(um.x+1.5708)));
    right.xy *= mm2(sin(time*0.7)*0.3);
    vec3 up = normalize(cross(right, eye));
    vec3 rd = normalize((p.x*right+p.y*up)*1.+eye);
    
    float rz = march(ro,rd);
    vec3 col = vec3(0.);
    
    if ( rz < FAR )
    {
        vec3 pos = ro+rz*rd;
        vec3 nor= normal(pos);
        vec3 ligt = normalize(vec3(-.2, 0.05, -0.2));
        
        float dif = clamp(dot( nor, ligt ), 0., 1.);
        float fre = pow(clamp(1.0+dot(nor,rd),0.0,1.0), 3.);
        vec3 brdf = 2.*vec3(0.10,0.11,0.1);
        brdf += 1.9*dif*vec3(.8,1.,.05);
        col = vec3(0.35,0.07,0.5);
        col = col*brdf + fre*0.5*vec3(.7,.8,1.);
    }
    col = clamp(col,0.,1.);
    col = pow(col,vec3(.9));
    col *= pow( 16.0*bp.x*bp.y*(1.0-bp.x)*(1.0-bp.y), 0.1);
    gl_FragColor = vec4( col, 1.0 );
}

/*
//#define time iGlobalTime
#define resolution iResolution
#define mouse iMouse


// https://www.shadertoy.com/view/Md23RK
float time;
vec3 pln;

float terrain(vec3 p)
{
	float nx=floor(p.x)*10.0+floor(p.z)*100.0,center=0.0,scale=2.0;
	vec4 heights=vec4(0.0,0.0,0.0,0.0);
	
	for(int i=0;i<5;i+=1)
	{
		vec2 spxz=step(vec2(0.0),p.xz);
		float corner_height = mix(mix(heights.x, heights.y, spxz.x),
								  mix(heights.w, heights.z, spxz.x),spxz.y);
		
		vec4 mid_heights=(heights+heights.yzwx)*0.5;
		
		heights =mix(mix(vec4(heights.x,mid_heights.x,center,mid_heights.w),
					     vec4(mid_heights.x,heights.y,mid_heights.y,center), spxz.x),
					 mix(vec4(mid_heights.w,center,mid_heights.z,heights.w),
						 vec4(center,mid_heights.y,heights.z,mid_heights.z), spxz.x), spxz.y);
		
		nx=nx*4.0+spxz.x+2.0*spxz.y;
		
		center=(center+corner_height)*0.5+cos(nx*20.0)/scale*30.0;
		p.xz=fract(p.xz)-vec2(0.5);
		p*=2.0;
		scale*=2.0;
	}
	
    
	float d0=p.x+p.z;
	
	vec2 plh=mix( mix(heights.xw,heights.zw,step(0.0,d0)),
                 mix(heights.xy,heights.zy,step(0.0,d0)), step(p.z,p.x));
	
	pln=normalize(vec3(plh.x-plh.y,2.0,(plh.x-center)+(plh.y-center)));
    
	if(p.x+p.z>0.0)
		pln.xz=-pln.zx;
	
	if(p.x<p.z)
		pln.xz=pln.zx;
	
	p.y-=center;
	return dot(p,pln)/scale;
}

void main()
{
	time=iGlobalTime*0.4;
	vec2 uv=(gl_FragCoord.xy / iResolution.xy)*2.0-vec2(1.0);
	uv.x*=iResolution.x/iResolution.y;
	
	float sc=(time+sin(time*0.2)*4.0)*0.8;
	vec3 camo=vec3(sc+cos(time*0.2)*0.5,0.7+sin(time*0.3)*0.4,0.3+sin(time*0.4)*0.8);
	vec3 camt=vec3(sc+cos(time*0.04)*1.5,-1.5,0.0);
	vec3 camd=normalize(camt-camo);
	
	vec3 camu=normalize(cross(camd,vec3(0.5,1.0,0.0))),camv=normalize(cross(camu,camd));
	camu=normalize(cross(camd,camv));
	
	mat3 m=mat3(camu,camv,camd);
	
	vec3 rd=m*normalize(vec3(uv,1.8)),rp;
	
	float t=0.0;
	
	for(int i=0;i<100;i+=1)
	{
		rp=camo+rd*t;
		float d=terrain(rp);
		if(d<4e-3)
			break;
		t+=d;
	}
    
	vec3 ld=normalize(vec3(1.0,0.6,2.0));
    
    vec3 color = mix(vec3(0.1,0.1,0.5)*0.4,vec3(1.0,1.0,0.8),pow(0.5+0.5*dot(pln,ld),0.7));
	color = mix(vec3(0.5,0.6,1.0),color,exp(-t*0.02));

	gl_FragColor = vec4(color,1.0);
}
*/