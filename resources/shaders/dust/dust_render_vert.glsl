#version 120
//#extension GL_ARB_point_parameters : enable
//#extension GL_ARB_point_sprite : enable

uniform sampler2D posMap;
uniform sampler2D velMap;

uniform float screenWidth;

varying float age;

void main()
{
	vec4 newVertexPos;
	vec4 dv;

	dv = texture2D( posMap, gl_MultiTexCoord0.st );
	
    age = texture2D( velMap, gl_MultiTexCoord0.st ).a;
	
    //scale vertex position to screen size
	newVertexPos = vec4(screenWidth * dv.x, screenWidth * dv.y, screenWidth * dv.z, 1.0);
	
    //adjust point size, increasing size kills performance
	gl_PointSize = 8.0 - (4.0 * age);

	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}

