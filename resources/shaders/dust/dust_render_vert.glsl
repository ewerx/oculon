#version 120
//#extension GL_ARB_point_parameters : enable
//#extension GL_ARB_point_sprite : enable

uniform sampler2D posMap;
uniform sampler2D velMap;
uniform sampler2D information;

uniform float screenWidth;
uniform float spriteWidth;

uniform bool audioReactive;
uniform vec3 trackAudio1;
uniform vec3 trackAudio2;
uniform vec3 trackAudio3;
uniform vec3 trackAudio4;

varying float age;
varying float maxAge;

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
    
    if (audioReactive) {
        gl_PointSize = clamp(200.0 * gl_MultiTexCoord0.s * gl_MultiTexCoord0.t * trackAudio1.x - (6.0 * spriteWidth * age), spriteWidth, spriteWidth * 8.0);
    }

	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}

