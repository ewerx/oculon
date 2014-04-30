#version 120 

uniform sampler2D posMap;
uniform sampler2D velMap;

uniform sampler2D spriteTex;
uniform vec4 colorBase;
uniform bool audioReactive;

varying float age;
varying float maxAge;
varying vec4 color;

void main()
{
	//vec4 colFac = vec4(age,age,age,age);
    vec4 colFac = texture2D(spriteTex, gl_PointCoord);
    colFac *= colorBase;
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
    
    if (audioReactive) {
        colFac.a *= (colFac.a + 10.0*color.a);
    }
    
	gl_FragColor = colFac;
}
