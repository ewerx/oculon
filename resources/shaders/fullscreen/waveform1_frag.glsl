//
//  rings_frag.glsl
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

uniform vec2 iResolution;     // viewport resolution (in pixels)
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
uniform int         iFormat1;

// ringset2
uniform float       iTime2;
uniform vec4        iColor2;
uniform float       iScale2;
uniform float       iFrequency2;
uniform float       iThickness2;
uniform float       iPower2;
uniform vec2        iCenter2;
uniform int         iFormat2;

// ringset3
uniform float       iTime3;
uniform vec4        iColor3;
uniform float       iScale3;
uniform float       iFrequency3;
uniform float       iThickness3;
uniform float       iPower3;
uniform vec2        iCenter3;
uniform int         iFormat3;

// ringset3
uniform float       iTime4;
uniform vec4        iColor4;
uniform float       iScale4;
uniform float       iFrequency4;
uniform float       iThickness4;
uniform float       iPower4;
uniform vec2        iCenter4;
uniform int         iFormat4;


#ifdef GL_ES
precision highp float;
#endif

vec4 calcRingSet(vec2 pos, float time, float frequency, float scale, float thickness, float power, int format)
{
    if (format == 1)
    {
        pos.y = pos.x;
    }
    else if (format == 2)
    {
        pos.x = pos.y;
    }
    
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
	//float r = frequency * sin(dist/scale*50.0); // neat in and out simulatanous
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

float invAr = iResolution.y / iResolution.x;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 pos1 = iCenter1 - uv; // pos in -0.5 to 0.5 range
    vec2 pos2 = iCenter2 - uv; // pos in -0.5 to 0.5 range
    vec2 pos3 = iCenter3 - uv; // pos in -0.5 to 0.5 range
    vec2 pos4 = iCenter4 - uv; // pos in -0.5 to 0.5 range
    // compensate for aspect ratio
    pos1.y *= invAr;
    pos2.y *= invAr;
    pos3.y *= invAr;
    pos4.y *= invAr;
    
    // color fade
    // fades intensity down towards edges of screen
    //float fade = 0.25*sin(PI*uv.y) + -0.5*sin(PI*uv.x);
    
    vec4 ringset1 = iColor1 * calcRingSet( pos1, iTime1, iFrequency1, iScale1, iThickness1, iPower1, iFormat1 );
    vec4 ringset2 = iColor2 * calcRingSet( pos2, iTime2, iFrequency2, iScale2, iThickness2, iPower2, iFormat2 );
    vec4 ringset3 = iColor3 * calcRingSet( pos3, iTime3, iFrequency3, iScale3, iThickness3, iPower3, iFormat3 );
    vec4 ringset4 = iColor4 * calcRingSet( pos4, iTime4, iFrequency4, iScale4, iThickness4, iPower4, iFormat4 );
	
	gl_FragColor = ringset1 * iColor1.w + ringset2 * iColor2.w + ringset3 * iColor3.w + ringset4 * iColor4.w;
}
