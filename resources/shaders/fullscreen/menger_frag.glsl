uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)

uniform int     iIterations;
uniform int     iMaxSteps;
uniform float     iFieldOfView;
uniform float     iScale;
uniform float     iJitter;
uniform float     iFudgeFactor;
uniform float     iPerspective;
uniform float     iMinDistance;
uniform float     iNormalDistance;

uniform float     iAmbientLight;
uniform float     iDiffuseLight;
uniform vec3      iLight1Dir;
uniform vec3      iLight2Dir;

uniform vec3      iOffset;
uniform vec3      iColor1;
uniform vec3      iColor2;

#define PI 3.1415927

vec2 rotate(vec2 v, float a) {
	return vec2(cos(a)*v.x + sin(a)*v.y, -sin(a)*v.x + cos(a)*v.y);
}

// Two light sources. No specular
vec3 getLight(in vec3 color, in vec3 normal, in vec3 dir) {
	vec3 lightDir = normalize(iLight1Dir);
	float diffuse = max(0.0,dot(-normal, lightDir)); // Lambertian
	
	vec3 lightDir2 = normalize(iLight2Dir);
	float diffuse2 = max(0.0,dot(-normal, lightDir2)); // Lambertian
	
	return
	(diffuse*iDiffuseLight)*(iColor1*color) +
	(diffuse2*iDiffuseLight)*(iColor2*color);
}


// DE: Infinitely tiled Menger IFS.
//
// For more info on KIFS, see:
// http://www.fractalforums.com/3d-fractal-generation/kaleidoscopic-%28escape-time-ifs%29/
float DE(in vec3 z)
{
	// enable this to debug the non-linear perspective
//	if (DebugiPerspective) {
//		z = fract(z);
//		float d=length(z.xy-vec2(0.5));
//		d = min(d, length(z.xz-vec2(0.5)));
//		d = min(d, length(z.yz-vec2(0.5)));
//		return d-0.01;
//	}
	// Folding 'tiling' of 3D space;
	z  = abs(1.0-mod(z,2.0));
    
	float d = 1000.0;
	for (int n = 0; n < iIterations; n++) {
		z.xy = rotate(z.xy,4.0+2.0*cos( iGlobalTime/8.0));
		z = abs(z);
		if (z.x<z.y){ z.xy = z.yx;}
		if (z.x< z.z){ z.xz = z.zx;}
		if (z.y<z.z){ z.yz = z.zy;}
		z = iScale*z-iOffset*(iScale-1.0);
		if( z.z<-0.5*iOffset.z*(iScale-1.0))  z.z+=iOffset.z*(iScale-1.0);
		d = min(d, length(z) * pow(iScale, float(-n)-1.0));
	}
	
	return d-0.001;
}

// Finite difference normal
vec3 getNormal(in vec3 pos) {
	vec3 e = vec3(0.0,iNormalDistance,0.0);
	
	return normalize(vec3(
                          DE(pos+e.yxx)-DE(pos-e.yxx),
                          DE(pos+e.xyx)-DE(pos-e.xyx),
                          DE(pos+e.xxy)-DE(pos-e.xxy)
                          )
                     );
}

// Solid color
vec3 getColor(vec3 normal, vec3 pos) {
	return vec3(1.0);
}


// Pseudo-random number
// From: lumina.sourceforge.net/Tutorials/Noise.html
float rand(vec2 co){
	return fract(cos(dot(co,vec2(4.898,7.23))) * 23421.631);
}

vec4 rayMarch(in vec3 from, in vec3 dir) {
	// Add some noise to prevent banding
	float totalDistance = iJitter*rand(gl_FragCoord.xy+vec2(iGlobalTime));
	vec3 dir2 = dir;
	float distance;
	int steps = 0;
	vec3 pos;
	for (int i=0; i < iMaxSteps; i++) {
		// Non-linear perspective applied here.
		dir.zy = rotate(dir2.zy,totalDistance*cos( iGlobalTime/4.0)*iPerspective);
		
		pos = from + totalDistance * dir;
		distance = DE(pos)*iFudgeFactor;
		totalDistance += distance;
		if (distance < iMinDistance) break;
		steps = i;
	}
	
	// 'AO' is based on number of steps.
	// Try to smooth the count, to combat banding.
	float smoothStep =   float(steps) + distance/iMinDistance;
	float ao = 1.1-smoothStep/float(iMaxSteps);
	
	// Since our distance field is not signed,
	// backstep when calc'ing normal
	vec3 normal = getNormal(pos-dir*iNormalDistance*3.0);
	
	vec3 color = getColor(normal, pos);
	vec3 light = getLight(color, normal, dir);
	color = (color*iAmbientLight+light)*ao;
	return vec4(color,1.0);
}

void main(void)
{
	// Camera position (eye), and camera target
	vec3 camPos = 0.5*iGlobalTime*vec3(1.0,0.0,0.0);
	vec3 target = camPos + vec3(1.0,0.0*cos(iGlobalTime),0.0*sin(0.4*iGlobalTime));
	vec3 camUp  = vec3(0.0,1.0,0.0);
	
	// Calculate orthonormal camera reference system
	vec3 camDir   = normalize(target-camPos); // direction for center ray
	camUp = normalize(camUp-dot(camDir,camUp)*camDir); // orthogonalize
	vec3 camRight = normalize(cross(camDir,camUp));
	
	vec2 coord =-1.0+2.0*gl_FragCoord.xy/iResolution.xy;
	coord.x *= iResolution.x/iResolution.y;
	
	// Get direction for this pixel
	vec3 rayDir = normalize(camDir + (coord.x*camRight + coord.y*camUp)*iFieldOfView);
	
	gl_FragColor = rayMarch(camPos, rayDir);
}




