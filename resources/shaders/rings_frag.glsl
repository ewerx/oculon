//
//  rings_frag.glsl
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

uniform vec3        iResolution;     // viewport resolution (in pixels)
//uniform float       iGlobalTime;     // shader playback time (in seconds)
//uniform float       iBackgroundAlpha;

// ringset1
uniform float       iTime1;
uniform vec4        iColor1;
uniform float       iScale1;
uniform float       iFrequency1;
uniform float       iThickness1;
uniform float       iPower1;
uniform vec2        iCenter1;

// ringset2
uniform float       iTime2;
uniform vec4        iColor2;
uniform float       iScale2;
uniform float       iFrequency2;
uniform float       iThickness2;
uniform float       iPower2;
uniform vec2        iCenter2;

// ringset3
uniform float       iTime3;
uniform vec4        iColor3;
uniform float       iScale3;
uniform float       iFrequency3;
uniform float       iThickness3;
uniform float       iPower3;
uniform vec2        iCenter3;


#ifdef GL_ES
precision highp float;
#endif

vec4 calcRingSet(vec2 pos, float time, float frequency, float scale, float thickness, float power)
{
    // TODO: figure out relationship between scale and frequency...
	float dist = length(pos);
    float dist2 = (dist*dist)/scale;
    
    // iPower contols desnity of lines as a function of distance to center
    // iPower range 0 - 1.0
    // iPower 0.5 = symetrical lines (square root of distance)
    // iPower > 0.5 = bulge in center
    // iPower < 0.5 = squeeze in center
    //
    // frequency controls # of rings visible
    // frequency range 1 - 100
	float r = frequency * pow(dist2,power);
	
	// experiments:
	//float r = frequency * sin(dist/scale*3.); // neat in and out simulatanous
	//float r = abs( tan(dist/scale*15.) * thickness);// WTF is this???
	//thickness *= sin(0.1*r); // relative thickness
    
	float z = abs(tan((r+time)) * thickness);
	
    vec4 texcol;
	texcol.x = z;
	texcol.y = z;
	texcol.z = z;
    texcol.w = 1.0; // TODO: support background alpha
    
    return texcol;
}

vec2 center = vec2(0.5,0.5);
float invAr = iResolution.y / iResolution.x;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 pos1 = iCenter1 - uv; // pos in -0.5 to 0.5 range
    vec2 pos2 = iCenter2 - uv; // pos in -0.5 to 0.5 range
    vec2 pos3 = iCenter3 - uv; // pos in -0.5 to 0.5 range
    // compensate for aspect ratio
    pos1.y *= invAr;
    pos2.y *= invAr;
    pos3.y *= invAr;
    // pos.y = pos.x ==> vertical lines
    // pos.x = pos.y ==> horizontal lines
    
    // color fade
    // fades intensity down towards edges of screen
    //float fade = 0.25*sin(PI*uv.y) + -0.5*sin(PI*uv.x);
    
    vec4 ringset1 = iColor1 * calcRingSet( pos1, iTime1, iFrequency1, iScale1, iThickness1, iPower1 );
    vec4 ringset2 = iColor2 * calcRingSet( pos2, iTime2, iFrequency2, iScale2, iThickness2, iPower2 );
    vec4 ringset3 = iColor3 * calcRingSet( pos3, iTime3, iFrequency3, iScale3, iThickness3, iPower3 );
	
	gl_FragColor = ringset1 * iColor1.w + ringset2 * iColor2.w + ringset3 * iColor3.w;
}
