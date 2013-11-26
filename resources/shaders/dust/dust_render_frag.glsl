#version 120 

uniform sampler2D posMap;
uniform sampler2D velMap;

uniform sampler2D spriteTex;


varying float age;
varying float maxAge;

void main()
{
	//vec4 colFac = vec4(age,age,age,age);
    vec4 colFac = texture2D(spriteTex, gl_PointCoord);
    //colFac.rgb *= texture2D( velMap, gl_TexCoord[0].st ).rgb;
    
    // darker with age
//	colFac.r *= (1.0-age);
//    colFac.g *= (1.0-age);
//    colFac.b *= (1.0-age);
    
    // alpha/color peaks at half-life
    float alpha;
    float aF = age - (maxAge * 0.4);
    alpha = 1.0 - abs(aF * 2.0);
    colFac *= alpha;
    
	gl_FragColor = colFac;
}
