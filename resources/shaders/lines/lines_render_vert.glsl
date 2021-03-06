#version 120

uniform sampler2D posMap;
uniform sampler2D velMap;
uniform sampler2D information;
uniform sampler2D colorMap;
uniform sampler2D intensityMap;

uniform float screenWidth;

uniform bool useColorMap;
uniform bool audioReactive;
uniform float gain;

varying float age;
varying float maxAge;
varying vec4 color;

void main()
{
	vec4 dv = texture2D( posMap, gl_MultiTexCoord0.st );
	
    // filter out the mass stored in last element
	vec4 newVertexPos  = vec4(dv.xyz,1.0);
	
    if (useColorMap)
    {
        color = texture2D( colorMap, gl_MultiTexCoord0.st );
    }
    else
    {
        color = vec4(1.0,1.0,1.0,0.0);
    }
    
    if (audioReactive)
    {
        // alpha from audio texture
        color.a = texture2D( intensityMap, vec2(gl_MultiTexCoord0.s,0.0) ).b;
    }
    else
    {
        color.a = 1.0;
    }

	gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}

