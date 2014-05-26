#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
//uniform sampler2D iChannel0;
//uniform sampler2D iChannel1;
//uniform vec2      iMouse;

/**
 
 DEMOS AND COLORS
 By @Eiyeron
 Based on Illogical from Matrefeytontias, plane deformations on TI-83/84 http://www.omnimaga.org/index.php?topic=17963.30
 And some tunnel effects.
 
 Use : Comment/Uncomment the defines as you wants, they'll enable/disable various effects in the shader.
 Note : that fork is mine work. I'm not on the same computer.
 **/
#ifdef GL_ES
precision mediump float;
#endif

#define SPEED 0.25
#define TWO_PI 6.28318530717958646
#define PI 3.14159265358979323
#define HALF_PI 1.570796326794896615
#define TAU (PI*2.0)

// #define CURVED_VIEW
// #define WOBBLE
// #define WAVE

// #define MOVE
// #define ROTATE
/// UNCOMMENT ONLY ONE OF THESE
// #define PLANE_EFFECT
// #define PLANE_EIGHT_EFFECT
#define TUNNEL_EFFECT
// #define TUNNEL_PLANE_EFFECT
// #define TELEPORT_EFFECT
// #define DISTORT_EFFECT
/// END OF WARNING

// #define CRAZY_CHECKERBOARD_MOTIF
// #define PATCH_MOTIF
// #define BLOB_MOTIF
//#define STRIPE_MOTIF
//#define STRIPE2_MOTIF
#define BLOBBY_WAVE_MOTIF
#define RED_BLUE_COLORS
//#define RAINBOW
// #define CRAZY_COLORS
//#define TESSELATED_COLORS

#define DISTANCE_FOG
#define BORDER_FOG
#define SCANLINE

#define RGB_DISTORSION
//#define THREE_D

#ifdef RGB_DISTORSION
#define RGB_D_VALUE 2.
#elif defined THREE_D
#define THREE_D_VALUE 2.
#endif


vec3 getColors( vec2 position )
{
	vec3 res = vec3(cos(position.x), sin(position.y), 1. - 0.5 * cos(iGlobalTime));
	return res;
}


// thanks to http://glsl.heroku.com/e#13822.0
float getRainbowValue(vec2 position)
{
	position.x=fract(0.16666 * abs(position.x));
	if(position.x>.5) position.x = 1.0-position.x;
	return smoothstep(.166666, .333333, position.x) * .5;
}

vec3 getRainbow(vec2 position) {
	return vec3(getRainbowValue(position + 3.),getRainbowValue(position + 1.), getRainbowValue(position + 5.));
}


float getCheckerboardColor( vec2 position ) {
	float  xpos = floor( 20.0 * position.x );
	float  ypos = floor( 10.0 * position.y );
	float col = mod( xpos, 2.0 );
	if( mod( ypos, 2.0 ) > 0.0 ) {
		col = cos(xpos*ypos + iGlobalTime * 5.);
	} else {
		col = sin(xpos*ypos + iGlobalTime * 5.);
		
	}
	return col;
}

vec3 shader(vec2 pos) {
	// Normalize coords from 0->1 with 0.5, 0.5 at center.
	vec2 position = TAU * pos.xy / iResolution.xy - PI;
	position.x *= iResolution.x / iResolution.y; // Correct aspect ratio.
	
#ifdef CURVED_VIEW
	// Curved effect.
	position.y -= 0.20*cos(position.x);
#endif
#ifdef WOBBLE
	// Wobble effect
	position.y += 0.2 * cos(position.x /2. + iGlobalTime * 0.37);
#endif
#ifdef WAVE
	position.y += 0.2*sin(sin(iGlobalTime)*4. + position.y*2. + iGlobalTime/8.);
#endif
	
	// Some calculations.
	float r = length(position);
	float a = atan(position.y, position.x);
	// Transition factor
	float factor = sin(iGlobalTime) /2. + 0.5;
    
#ifdef ROTATE
	a+= sin(iGlobalTime /20.);
#endif
	
	// THe magic happens right here.
	float u = position.x,v = position.y;
#if defined PLANE_EFFECT
	u = position.x/abs(position.y) ;
	v = 1./abs(position.y);
#elif defined PLANE_EIGHT_EFFECT
	u = position.x/abs(position.y) ;
	v = cos(u)/abs(position.y);
#elif defined TUNNEL_EFFECT
	u = a;
	v = 1./r;
#elif defined TUNNEL_PLANE_EFFECT
	u = factor*(position.x/abs(position.y)) + (1.-factor)*(a);
	v = factor*(1./abs(position.y)) + (1.-factor)*(1./r);
#elif defined TELEPORT_EFFECT
	u = a;
	v = 1./(r + sin(r + iGlobalTime));
#elif defined DISTORT_EFFECT
	u = r * cos(a + r);
	v = r * sin(a + r);
#endif
	
	vec2 p = vec2(u, v)/PI*3.;
#ifdef MOVE
	// Make that move! :p
	p += vec2( SPEED * cos(iGlobalTime), SPEED * iGlobalTime );
#endif
	
	vec3 color = vec3(1.0);
#ifdef RAINBOW
	color = getRainbow(p);
#endif
#ifdef CRAZY_COLORS
	// Get some colors right now
	color *= getColors(p);
#endif
	
#ifdef RED_BLUE_COLORS
	color *= vec3(color.r*1.0, -color.g*1.0,-color.b*4.0);
#endif
#ifdef TESSELATED_COLORS
	// Thanks http://glsl.heroku.com/e#13699.0
	color *= vec3(sin(dot(p,position)), cos(dot(p,position)),sin(dot(p,position)));
#endif
#ifdef CRAZY_CHECKERBOARD_MOTIF
	// Get the checkerboard motif
	color *= vec3(getCheckerboardColor( p ));
#endif
#ifdef PATCH_MOTIF
	color *= cos( p.x * cos( iGlobalTime / 8.0 ) * 5. ) + cos( p.y * cos( iGlobalTime / 85.0 ) * 3.);
	color *= cos( p.x * cos( iGlobalTime / 15.0 ) * 7. ) - cos( p.y * cos( iGlobalTime / 125.0 ) * 4.);
#endif
#ifdef BLOB_MOTIF
	float col = 0.0;
	for(float i = 0.0; i < 5.; i++)
	{
	  	float a = i * (TAU/5.) * 61.95;
		col += cos(TAU*(p.y * cos(a) + p.x * sin(a) /*+ mouse.y +i*mouse.x*/ + sin(iGlobalTime*0.004)*100.0 ));
	}
	col /= 3.0;
	color *= vec3(col);
#endif
#ifdef STRIPE_MOTIF
	color *= float(mod(p.x+p.y + iGlobalTime, 2.0) > 1.);
#endif
	// Use v and r to create a little distance fog.
#ifdef STRIPE2_MOTIF
	float pat = abs(sin(10.0*mod(-abs(p.x*p.y + a), p.x)));
	color *= 2.*pat;
#endif
#ifdef BLOBBY_WAVE_MOTIF
	float t = iGlobalTime / 1.5;
	vec2 glp = (p.xy) * vec2(TWO_PI, PI) - vec2(PI, HALF_PI);
	vec2 glm = vec2(sin(t), cos(t)) * vec2(TWO_PI, PI) - vec2(PI, HALF_PI);
	float multp = cos(glp.y);
	vec3 posp = vec3(multp * cos(glp.x), multp * sin(glp.x), sin(glp.y));
	float multm = cos((glm.y +sin(t))/2.);
	vec3 posm = vec3(multm * cos(glm.y), multm * sin(glm.y), sin(glm.x));
	color *= vec3(dot(posp, posm));
    
#endif
	
#ifdef DISTANCE_FOG
	color *= 1./v;
#endif
#ifdef BORDER_FOG
	color *= 2.-r;
#endif
	return color;
	
}

void main( void ) {
    
    vec2 resolution = iResolution;
    
    
	vec3 color;
#ifdef RGB_DISTORSION
	color = vec3(shader(gl_FragCoord.xy + vec2(RGB_D_VALUE, 0.)).r, shader(gl_FragCoord.xy + vec2(0.,-RGB_D_VALUE)).g, shader(gl_FragCoord.xy + vec2(0.,RGB_D_VALUE)).b);
#elif defined THREE_D
	vec2 position = 2.0 * gl_FragCoord.xy / resolution.xy - 1.0;
	position.x *= resolution.x / resolution.y; // Correct aspect ratio.
	color = vec3(shader(gl_FragCoord.xy + vec2(THREE_D_VALUE*length(position),0.)).r,
                 shader(gl_FragCoord.xy).g,
                 shader(gl_FragCoord.xy + vec2(-THREE_D_VALUE*length(position),0.)).b);
#else
	color = shader(gl_FragCoord.xy);
#endif
#ifdef SCANLINE
	// Little scanline effect
	color *= 1. * mod(gl_FragCoord.y, 2.);
#endif
    
	// Let's fill the screen with eyecandy!
	gl_FragColor = vec4( color, 1.0 );
}
