#version 120

uniform float	scale;

uniform sampler2D posMap;
uniform sampler2D velMap;
uniform sampler2D information;

void main() {
    vec4 pos = texture2D( posMap, gl_MultiTexCoord0.st );
    vec3 vertex = vec3(gl_ModelViewMatrix * vec4(pos.xyz,1.0));
	
	// set position
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos.xyz,1.0);
    gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor  = gl_Color;
}