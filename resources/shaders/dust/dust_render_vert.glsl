#version 120
//#extension GL_ARB_point_parameters : enable
//#extension GL_ARB_point_sprite : enable

uniform sampler2D posMap;
uniform sampler2D velMap;
uniform sampler2D information;
uniform sampler2D intensityMap;

uniform float screenWidth;
uniform float spriteWidth;

uniform bool audioReactive;
uniform float gain;
uniform vec3 trackAudio1;
uniform vec3 trackAudio2;
uniform vec3 trackAudio3;
uniform vec3 trackAudio4;

varying float age;
varying float maxAge;
varying vec4 color;

void main()
{
	vec4 newVertexPos;
	vec4 dv;

	dv = texture2D( posMap, gl_MultiTexCoord0.st );
	
    age = texture2D( velMap, gl_MultiTexCoord0.st ).a;
    maxAge = texture2D( information, gl_MultiTexCoord0.st ).g;
	
    // scale vertex position to screen size
	newVertexPos = vec4(screenWidth * dv.x, screenWidth * dv.y, screenWidth * dv.z, 1.0);
	
    // particles shrink with age
	gl_PointSize = 6.0 * spriteWidth - (6.0 * spriteWidth * age);
    
    color = vec4(1.0,1.0,1.0,0.0);
    if (audioReactive)
    {
        // size based on audio level
        float pointSize = gl_PointSize;
        pointSize = pointSize * 0.5 + pointSize * texture2D( intensityMap, vec2(gl_MultiTexCoord0.s,0.0) ).x * gain;
        gl_PointSize = min( pointSize, 15.0 );
        
        // alpha from audio texture
        color.a = texture2D( intensityMap, vec2(gl_MultiTexCoord0.s,0.0) ).x * gain;
    }
    else
    {
        color.a = 1.0;
    }

	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}

