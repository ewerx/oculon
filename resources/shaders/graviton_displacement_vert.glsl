#version 120
uniform sampler2D displacementMap;
uniform sampler2D velocityMap;
uniform vec4 eyePos;
uniform float scale;

varying vec4 color;

// define a few constants here, for faster rendering
const float LOG_BASE10 = 1.0 / log2(10.0);

const float SIZE = 90.0;			// the higher the value, the bigger the stars will be
const float SIZE_MODIFIER = 1.5;	// the lower the value, the more stars are visible

float log10( float n ) {
	return log2(n) * LOG_BASE10;
}

const float constAtten  = 0.01;
const float linearAtten = 0.01;
const float quadAtten   = 0.01;

void main()
{
    vec4 position=gl_ModelViewMatrix*gl_Vertex;
    vec4 cameraloc=gl_ModelViewMatrix*eyePos;
    float d=distance(vec3(cameraloc),vec3(position));
////    float a_hsml=gl_Normal.x;
////    float pointSize=1800.*a_hsml/d; // <- point diameter in
////    //pixels (drops like sqrt(1/r^2))
////    gl_PointSize=pointSize;
//    
//    vec3	vertex = vec3(gl_ModelViewMatrix * gl_Vertex);
//	float	d = length(vertex);
//    float attn = inversesqrt(constAtten + linearAtten*d + quadAtten*d*d);
//    
//    gl_PointSize = 2.0 * attn;
    
    // retrieve absolute magnitude from texture coordinates
	float magnitude = 5.0;//gl_MultiTexCoord0.x;
    
	// calculate apparent magnitude based on distance
	float apparent = magnitude - 1.0 * (1.0 - log10(d));
    
	// calculate point size based on apparent magnitude
    gl_PointSize = 1.0 * SIZE * pow(SIZE_MODIFIER, 1.0 - apparent);
    
    // determine color
//	float brightness = clamp((16. + (1.0 - apparent)) / 16., 0.0, 1.0);
	//gl_FrontColor = gl_Color * pow(brightness, 1.5);
    
    //using the displacement map to move vertices
	vec4 pos = texture2D( displacementMap, gl_MultiTexCoord0.xy );
    //color = texture2D( velocityMap, gl_MultiTexCoord0.xy ) + 0.01;
    color = vec4(0.1,0.1,0.1,0.1);
    
	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1.0) ;
    
    
}