uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec2      iMouse;

// Weather. By David Hoskins, May 2014.
// @ https://www.shadertoy.com/view/4dsXWn

// Who needs mathematically correct simulations?! :)
// It ray-casts to the bottom layer then steps through to the top layer.
// It uses the same number of steps for all positions.
// The larger steps at the horizon don't cause problems as they are far away.
// So the detail is where it matters.
// Unfortunately this can't be used to go through the cloud layer,
// but it's fast and has a massive view distance.

vec3 sunLight  = normalize( vec3(  0.35, 0.17,  0.3 ) );
vec3 sunColour = vec3(1.0, .6, .25);
float gTime;
float cloudy;

#define cloudLower 2000.0
#define cloudUpper 2800.0

#define TEXTURE_NOISE


//--------------------------------------------------------------------------
float Hash( float n )
{
	return fract(sin(n)*43758.5453);
}

//--------------------------------------------------------------------------
float Noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    float res = mix(mix( Hash(n+  0.0), Hash(n+  1.0),f.x),
                    mix( Hash(n+ 57.0), Hash(n+ 58.0),f.x),f.y);
    return res;
}
//--------------------------------------------------------------------------
float Hash(in vec3 p)
{
    return fract(sin(dot(p,vec3(37.1,61.7, 12.4)))*3758.5453123);
}

//--------------------------------------------------------------------------
#ifdef TEXTURE_NOISE
float Noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}
#else
float Noise(in vec3 p)
{
    vec3 i = floor(p);
	vec3 f = fract(p);
	f *= f * (3.0-2.0*f);
    
    return mix(
               mix(mix(Hash(i + vec3(0.,0.,0.)), Hash(i + vec3(1.,0.,0.)),f.x),
                   mix(Hash(i + vec3(0.,1.,0.)), Hash(i + vec3(1.,1.,0.)),f.x),
                   f.y),
               mix(mix(Hash(i + vec3(0.,0.,1.)), Hash(i + vec3(1.,0.,1.)),f.x),
                   mix(Hash(i + vec3(0.,1.,1.)), Hash(i + vec3(1.,1.,1.)),f.x),
                   f.y),
               f.z);
}
#endif

//--------------------------------------------------------------------------
float FBM( vec3 p )
{
	p *= .5;
    float f;
	
	f = 0.5000 * Noise(p); p = p * 3.02; p.y -= gTime*.2;
	f += 0.2500 * Noise(p); p = p * 3.03; p.y += gTime*.06;
	f += 0.1250 * Noise(p); p = p * 3.01;
	f += 0.0625   * Noise(p); p =  p * 3.03;
	f += 0.03125  * Noise(p); p =  p * 3.02;
	f += 0.015625 * Noise(p);
    return f;
}

//--------------------------------------------------------------------------
float SeaFBM( vec2 p )
{
    float f;
	f = sin(sin(p.x *.22) + cos(p.y *.24)+p.x*.05+p.y*.03);
    f += 0.5000 * Noise(p); p =  p * 2.12;
    f += 0.2500 * Noise(p); p =  p * 2.07;
    f += 0.1250 * Noise(p); p =  p * 2.13;
    f += 0.0625 * Noise(p);
    
	return f;
}

//--------------------------------------------------------------------------
float Map(vec3 p)
{
	float h = FBM(p);
	return h-cloudy-.45;
}

//--------------------------------------------------------------------------
float SeaMap(in vec2 pos)
{
	pos *= .008;
	return SeaFBM(pos) * 15.0;
}

//--------------------------------------------------------------------------
vec3 SeaNormal( in vec3 pos, in float d)
{
	float p = .025 * d * d / iResolution.x;
	vec3 nor  	= vec3(0.0,		    SeaMap(pos.xz), 0.0);
	vec3 v2		= nor-vec3(p,		SeaMap(pos.xz+vec2(p,0.0)), 0.0);
	vec3 v3		= nor-vec3(0.0,		SeaMap(pos.xz+vec2(0.0,-p)), -p);
	nor = cross(v2, v3);
	return normalize(nor);
}

//--------------------------------------------------------------------------
// Grab all sky information for a given ray from camera
vec3 GetSky(in vec3 pos,in vec3 rd)
{
	float sunAmount = max( dot( rd, sunLight), 0.0 );
	// Do the blue and sun...
	vec3  sky = mix(vec3(.15, .2, .4), vec3(.25, .5, .7), 1.0-rd.y);
	sky = sky + sunColour * min(pow(sunAmount, 1500.0) * 2.0, 1.0);
	sky = sky + sunColour * min(pow(sunAmount, 10.0) * .75, 1.0);
	
	// Find the start and end of the cloud layer...
	float beg = ((cloudLower-pos.y) / rd.y);
	float end = ((cloudUpper-pos.y) / rd.y);
	
	// Start position...
	vec3 p = vec3(pos.x + rd.x * beg, 0.0, pos.z + rd.z * beg);
    
	// Trace clouds through that layer...
	float d = 0.0;
	float add = (end-beg) / 45.0;
	// Horizon fog is just thicker clouds...
	vec2 shade = vec2(0.0, pow(1.0-rd.y,30.) * .2);
	vec2 shadeSum = vec2(0.07, .0);
	float difference = cloudUpper-cloudLower;
	// Shade is a function of transparency...
	for (int i = 0; i < 45; i++)
	{
		if (shadeSum.y >= 1.0) continue;
		vec3 pos = p + rd * d;
		float h = Map(pos * .001);
		shade.y += max(-h, 0.0) * .07;
		shade.x = mix((pos.y / difference) * shade.y, 1.0, max((.25-shade.y) * .05, 0.0));
		shadeSum += shade * (1.0 - shadeSum.y);
		d += add;
	}
	shadeSum.y = min(shadeSum.y, 1.0);
	
	vec3 clouds = mix(vec3(shadeSum.x), sunColour, clamp(pow(shadeSum.x, 3.5), 0.0, .25));
	clouds += min((1.0-shadeSum.y) * pow(sunAmount, 3.0), 1.0);
    
	sky = mix(sky, min(clouds, 1.0), shadeSum.y);
    
	return clamp(sky, 0.0, 1.0);
}

//--------------------------------------------------------------------------
vec3 GetSea(in vec3 pos,in vec3 rd)
{
	vec3 sea;
	float d = -pos.y/rd.y;
	vec3 p = vec3(pos.x + rd.x * d, 0.0, pos.z + rd.z * d);
	
	float dis = length(p-pos);
	vec3 nor = SeaNormal(p, dis);
    
	vec3 ref = reflect(rd, nor);
	ref.y = max(ref.y, 0.0015);
	sea = GetSky(p, ref);
	
	sea = mix(sea*.6, vec3(.15, .3, .4), .2);
	
	float glit = max(dot(ref, sunLight), 0.0);
	sea += sunColour * pow(glit, 220.0) * max(-cloudy*100.0, 0.0);
	
	return sea;
}

//--------------------------------------------------------------------------
vec3 CameraPath( float t )
{
    return vec3(4000.0 * sin(.16*t)+2290.0, 0.0, 4000.0 * cos(.155*t));
}

//--------------------------------------------------------------------------
void main(void)
{
	float m = (iMouse.x/iResolution.x)*30.0;
	gTime = iGlobalTime*.5 + m + 69.;
	cloudy = cos(gTime * .27+.37) * .25;
	
    vec2 xy = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = (-1.0 + 2.0 * xy) * vec2(iResolution.x/iResolution.y,1.0);
	
	vec3 cameraPos = CameraPath(gTime - 2.0);
	vec3 camTar	   = CameraPath(gTime - 1.0);
	camTar.y = cameraPos.y = sin(gTime) * 200.0 + 300.0;
	camTar.y += 300.0;
	
	float roll = .1 * sin(gTime * .25);
	vec3 cw = normalize(camTar-cameraPos);
	vec3 cp = vec3(sin(roll), cos(roll),0.0);
	vec3 cu = cross(cw,cp);
	vec3 cv = cross(cu,cw);
	vec3 dir = normalize(uv.x*cu + uv.y*cv + 1.3*cw);
	mat3 camMat = mat3(cu, cv, cw);
    
	vec3 col;
    
	if (dir.y > 0.0)
	{
		col = GetSky(cameraPos, dir);
	}else
	{
		col = GetSea(cameraPos, dir);
	}
	
	// Do the lens flares...
	float bri = dot(cw, sunLight) * 2.7 * clamp(-cloudy+.2, 0.0, .2);
	if (bri > 0.0)
	{
		vec2 sunPos = vec2( dot( sunLight, cu ), dot( sunLight, cv ) );
		vec2 uvT = uv-sunPos;
		uvT = uvT*(length(uvT));
		bri = pow(bri, 6.0)*.6;
        
		float glare1 = max(1.0-length(uvT+sunPos*2.)*2.0, 0.0);
		float glare2 = max(1.0-length(uvT+sunPos*.5)*4.0, 0.0);
		uvT = mix (uvT, uv, -2.3);
		float glare3 = max(1.0-length(uvT+sunPos*5.0)*1.2, 0.0);
        
		col += bri * sunColour * vec3(1.0, .5, .2)  * pow(glare1, 10.0)*25.0;
		col += bri * vec3(.8, .8, 1.0) * pow(glare2, 8.0)*9.0;
		col += bri * sunColour * pow(glare3, 4.0)*10.0;
	}
	
	vec2 st =  uv * vec2(.0+(xy.y+2.0)*.3, .02)+vec2(gTime*.5+xy.y*.4, gTime*.2);
	// Rain...
	float f = texture2D(iChannel0, st).y * texture2D(iChannel0, st*.773).x * 1.55;
	float rain = clamp(cloudy-.15, 0.0, 1.0);
	f = clamp(pow(abs(f), 10.0) * 3.0 * (rain*rain*125.0), 0.0, (xy.y+.1)*.6);
	col = mix(col, vec3(0.25, .25, .2), f);
    
	// Don't gamma too much to keep it looking moody...
	col = pow(col, vec3(.75));
	col *= .45+0.54*pow(70.0*xy.x*xy.y*(1.0-xy.x)*(1.0-xy.y), 0.1 );
	gl_FragColor=vec4(col, 1.0);
}

//--------------------------------------------------------------------------
