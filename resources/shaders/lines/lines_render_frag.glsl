#version 120 

uniform sampler2D posMap;
uniform sampler2D velMap;
uniform sampler2D colorMap;

uniform vec4 colorBase;
uniform bool audioReactive;

varying float age;
varying float maxAge;
varying vec4 color;


void main()
{
	vec4 finalColor = colorBase;
    if (audioReactive) {
        finalColor.a *= (finalColor.a + 10.0*color.a);
    }
    
	gl_FragColor = finalColor;
}
