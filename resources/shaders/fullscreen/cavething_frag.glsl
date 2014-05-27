#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec2      iMouse;

#define time iGlobalTime
#define resolution iResolution
#define mouse iMouse

// https://www.shadertoy.com/view/MsfSW4#
#define STEPS 128
#define PRECISION 0.0001
#define DEPTH 50.0
#define PI 3.14159265359

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Object {
	vec3 position;
	float distance;
	int material;
};

vec3 eye = vec3(0,0,0);
vec3 light = vec3(0,0,1);
Object scene(vec3);

float t = iGlobalTime*0.7+15.0;
float sdCylinder(vec3 p, float r) { return length(p.yx)-r; }

mat3 rotZ(float a){float s=sin(a);float c=cos(a);return mat3(c,-s,0,s,c,0,0,0,1);}
mat3 rotX(float a){float s=sin(a);float c=cos(a);return mat3(1,0,0,0,c,s,0,-s,c);}
mat3 rotY(float a){float s=sin(a);float c=cos(a);return mat3(c,0,-s,0,1,0,s,0,c);}

vec3 getNormal(vec3 p)
{
	vec2 e = vec2(PRECISION,0);
	vec3 n = normalize(vec3(
                            scene(p+e.xyy).distance - scene(p-e.xyy).distance,
                            scene(p+e.yxy).distance - scene(p-e.yxy).distance,
                            scene(p+e.yyx).distance - scene(p-e.yyx).distance
                            ));
	
	return n;
}

Object scene(vec3 p)
{
	vec3 pc = vec3(p.x+sin(p.z-t*0.1),p.y+sin(p.z),p.z);
	pc *= vec3(sin(pc.z-pc.y*-pc.x)*sin(2.0*pc.x+pc.z)*sin(pc.z+2.0*pc.y));
	
	float d = max(sdCylinder(pc,0.2),-sdCylinder(pc,0.1));
	return Object(p, d, 0);
}

Object march(Ray ray)
{
	float t = 0.0; Object o;
	
	for(int i=0; i < STEPS; i++)
	{
		o = scene(ray.origin + ray.direction * t);
		if (o.distance < PRECISION || t > DEPTH) { break; }
		t += o.distance;
	}
	
	return o;
}


Ray lookAt(vec3 o, vec3 t)
{
	vec2 uv = (2.0 * gl_FragCoord.xy - iResolution.xy) / iResolution.xx;
	vec3 dir = normalize(t-o), up = vec3(0,1,0), right = cross(up,dir);
	return Ray(o,normalize(right*uv.x + cross(dir,right)*uv.y + dir));
}

void main()
{
	eye.z   += t;
	light.z += t;
	
	eye.y = light.y = -sin(eye.z);
	eye.x = light.x = -sin(eye.z-t*0.1);
	
	Ray ray = lookAt(eye, light);
	ray.direction *= rotX(-cos(eye.z+0.4));
	ray.direction *= rotY(cos(eye.z-t*0.1));
	
	Object o = march(ray);
	
	vec3 col = texture2D(iChannel0,o.position.xz*4.0).rgb;
	vec3 bm = normalize(texture2D(iChannel1,o.position.xz*8.0).rgb);
	vec3 n = -normalize(getNormal(o.position)*bm);
    
	float distance = length(light-o.position);
	float diff = max(dot(n, reflect(normalize(light-o.position),n)),0.0);
	float spec = pow(diff,50.0);
    
	col += diff*0.2 + spec*0.3;
	col *= min(1.0, 1.0/distance);
    
	gl_FragColor = vec4(col,1.0);
}