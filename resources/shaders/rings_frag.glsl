uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform float        iRings;
uniform float       iTimeScale;
uniform float       iSmoothing;
uniform vec4        iColor1;
uniform vec4        iColor2;
uniform float       iIntervals;
uniform int         iColorMode;
uniform vec3        iCoefficients;

// based on https://www.shadertoy.com/view/XsXGD8

//const float velocity=-1.;
//const float b = 0.003;		//size of the smoothed border


#ifdef GL_ES
precision highp float;
#endif
#define EFFECT0 // 0..2

void main(void)
{
    float scale  = cos(iGlobalTime*iTimeScale) * 0.01 + 0.011;
    vec2 offset  = iResolution.xy*0.5;
    //		           + vec2(sin(iGlobalTime*0.3), cos(iGlobalTime*0.6))*100.0;
    
    
    vec2 pos = (gl_FragCoord.xy-offset) / iResolution.xy;
    float aspect = iResolution.x /iResolution.y;
    pos.x = pos.x*aspect;
    
    float dist = length(pos);
    float dist2 = (dist*dist)/scale;
    
    //
    float cx = iCoefficients.x * 0.1;
    float cy = iCoefficients.y * 0.1;
    float cz = iCoefficients.z * 0.1;
    float rings1 = abs( tan(dist/scale*cx*iGlobalTime) * sin(dist*0.01/scale) * 0.1 );
    float rings2 = abs( tan(dist/scale*cy*iGlobalTime) * 0.1);
    float rings3 = abs( cos(dist2*cz*iGlobalTime) * cos(dist2*0.0067*iGlobalTime));
    
    vec4 color;
    if (iColorMode == 0)
    {
        float grey = rings1+rings2+rings3;
        color = vec4(grey,grey,grey,1.0);
    }
    else if (iColorMode == 1)
    {
        color = vec4(rings1,
                     rings2,
                     rings3,
                     1.0);
    }
    else if (iColorMode == 2) {
        // space eye
        
        color = vec4( abs( sin(pos.y*dist2*0.1/scale)),
                     abs( sin(pos.y*dist2*0.3/scale)  * sin(dist*1.1/scale)),
                     abs( sin(pos.y*dist2*0.2/scale)) * cos(dist*0.05/scale),
                     1.0);
    }
    else if (iColorMode == 3)
    {
        //float cx = smoothstep( 0., iGlobalTime, iCoefficients.x );
        float grey = abs( cos(dist2*iCoefficients.x) * cos(dist2*iCoefficients.y) * cos(dist2*iCoefficients.z));
        color = vec4(grey, grey, grey, 1.0);
        
    }
    
    
    gl_FragColor = color;
}

//void main()
//{
//    float b = iSmoothing;
//	vec2 position = gl_FragCoord.xy/iResolution.xy;
//    float aspect = iResolution.x/iResolution.y;
//	position.x *= aspect;
//	float dist = distance(position, vec2(aspect*0.5, 0.5));
//	float offset=iGlobalTime*iTimeScale;
//	float conv=iRings;
//	float v=dist*conv-offset;
//	float ringr=floor(v);
//	float color=smoothstep(-b, b, abs(dist- (ringr+float(fract(v)>0.5)+offset)/conv));
//	if(mod(ringr,iIntervals)==1.)
//		color=1.-color;
//	gl_FragColor = color * iColor1;//vec4(color, color, color, 1.);
//}

