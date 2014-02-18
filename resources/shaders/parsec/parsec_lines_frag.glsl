#version 120

uniform sampler2D tex0;
uniform sampler2D tex1;

uniform float time;
uniform float aspect;

void main() 
{
	vec4 finalColor = vec4(1.0,1.0,1.0,1.0);
//    if (audioReactive) {
//        finalColor.a *= (finalColor.a + 10.0*color.a);
//    }
    
	gl_FragColor = finalColor;
}