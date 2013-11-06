uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float     iBackgroundAlpha;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform vec4      iColor3;
uniform float     iTimeScale;
uniform float     iZoom;
uniform float     iZoomScale;
uniform float     iPower;
uniform bool      iReverse;

// based on https://www.shadertoy.com/view/ldX3zr

vec2 center = vec2(0.5,0.5);
float speed = 0.01;
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
    
    vec4 texcol;
    
	float x = pos.x;//(center.x-uv.x);
	float y = pos.y;//(center.y-uv.y) *invAr;

    // iPower contols desnity of lines as a function of distance to center
    // iPower range 0 - 1.0
    // iPower 0.5 = symetrical lines (square root of distance)
    // iPower > 0.5 = bulge in center
    // iPower < 0.5 = squeeze in center
    //
    // zoom controls # of rings visible
    // zoom range 1 - 100 (iZoomScale 1-10, iZoom 1-10)
    float r = 10. * iZoom * iZoomScale * pow((x*x + y*y),iPower);
    
    // reverse motion
    // TODO: make it variable by making zoomscale range -10 to +10 ?
    if (iReverse)
    {
        r *= -1.0;
    }
    
	float z = 0.5 + 0.5*sin((r+iGlobalTime*iTimeScale));
	
	texcol.x = z;
	texcol.y = z;
	texcol.z = z;
    texcol.w = iBackgroundAlpha;
	
	gl_FragColor = vec4(iColor1*texcol);
}


