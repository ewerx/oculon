#version 120 

uniform sampler2D posMap;
uniform sampler2D velMap;

uniform sampler2D spriteTex;


varying float age;

void main()
{
	//vec4 colFac = vec4(age,age,age,age);
    vec4 colFac = texture2D(spriteTex, gl_PointCoord);
    //vec4 colFac = vec4(1.0,1.0,1.0,1.0);
    //colFac.rgb *= texture2D( posTex, gl_TexCoord[0].st ).rgb;

	colFac *= (1.0-age);
    
	gl_FragColor = colFac;
}
