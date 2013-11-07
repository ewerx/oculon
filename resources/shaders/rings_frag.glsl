

uniform vec3        iResolution;     // viewport resolution (in pixels)
//uniform float       iGlobalTime;     // shader playback time (in seconds)
//uniform float       iBackgroundAlpha;

// ringset1
uniform float       iTime1;
uniform vec4        iColor1;
uniform float       iScale1;
uniform float       iZoom1;
uniform float       iThickness1;
uniform float       iPower1;

// ringset2
uniform float       iTime2;
uniform vec4        iColor2;
uniform float       iScale2;
uniform float       iZoom2;
uniform float       iThickness2;
uniform float       iPower2;

// ringset3
uniform float       iTime3;
uniform vec4        iColor3;
uniform float       iScale3;
uniform float       iZoom3;
uniform float       iThickness3;
uniform float       iPower3;


#ifdef GL_ES
precision highp float;
#endif

vec4 calcRingSet(vec2 pos, float time, float zoom, float scale, float thickness, float power)
{
    // TODO: figure out relationship between scale and zoom...
	float dist = length(pos);
    float dist2 = (dist*dist)/scale;
    
    // iPower contols desnity of lines as a function of distance to center
    // iPower range 0 - 1.0
    // iPower 0.5 = symetrical lines (square root of distance)
    // iPower > 0.5 = bulge in center
    // iPower < 0.5 = squeeze in center
    //
    // zoom controls # of rings visible
    // zoom range 1 - 100
	float r = zoom * pow(dist2,power);
	
	// experiments:
	//float r = zoom * sin(dist/scale*3.); // neat in and out simulatanous
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
    vec2 pos = center - uv; // pos in -0.5 to 0.5 range
    pos.y *= invAr; // compensate for aspect ratio
    // pos.y = pos.x ==> vertical lines
    // pos.x = pos.y ==> horizontal lines
    
    // color fade
    // fades intensity down towards edges of screen
    //float fade = 0.25*sin(PI*uv.y) + -0.5*sin(PI*uv.x);
    
    vec4 ringset1 = iColor1 * calcRingSet( pos, iTime1, iZoom1, iScale1, iThickness1, iPower1 );
    vec4 ringset2 = iColor2 * calcRingSet( pos, iTime2, iZoom2, iScale2, iThickness2, iPower2 );
    vec4 ringset3 = iColor3 * calcRingSet( pos, iTime3, iZoom3, iScale3, iThickness3, iPower3 );
	
	gl_FragColor = ringset1 + ringset2 + ringset3;
}
