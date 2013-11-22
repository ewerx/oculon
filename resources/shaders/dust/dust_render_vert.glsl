#version 120
//#extension GL_ARB_point_parameters : enable
//#extension GL_ARB_point_sprite : enable

uniform sampler2D posMap;
uniform sampler2D velMap;

uniform float screenWidth;
uniform float spriteWidth;

varying float age;

void main()
{
	vec4 newVertexPos;
	vec4 dv;

	dv = texture2D( posMap, gl_MultiTexCoord0.st );
	
    age = texture2D( velMap, gl_MultiTexCoord0.st ).a;
	
    // scale vertex position to screen size
	newVertexPos = vec4(screenWidth * dv.x, screenWidth * dv.y, screenWidth * dv.z, 1.0);
	
    // particles shrink with age
	gl_PointSize = 4.0 * spriteWidth - (4.0 * spriteWidth * age);

	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}

