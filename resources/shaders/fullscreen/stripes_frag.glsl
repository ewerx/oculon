uniform vec2 iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float timeScale;
uniform float countScale;
uniform vec4 color1;
uniform vec4 color2;

void main(void)
{
    float divs = countScale; // 12.0
    
    vec2 div = vec2( divs, divs*iResolution.y/iResolution.x );
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv.y -= 0.5;									// center on screen
    float b = 4.0*divs/iResolution.x;			// blur over 2.4 pixels
    vec2 xy = div*uv;
    
    vec2 S;
    S.x = (xy.x + xy.y)*(xy.x - xy.y)*0.5;		// "velocity potential"
    S.y = xy.x*xy.y;							// stream function
    S.x += iGlobalTime*3.0;						// animate stream
    
    vec2 sxy = abs( sin(3.14159*S) );
    float a = sxy.x * sxy.y;					// combine sine waves using product
    
    a = smoothstep( 0.8-b, 0.8+b, a );			// threshold
    
    float c = sqrt( a );						// correct for gamma
    gl_FragColor = vec4(c, c, c, 1.0);
    //	gl_FragColor = vec4(c, sxy.x, sxy.y, 1.0);
}

//void main(void)
//{
//	float stripeCount = countScale*10.0 + countScale*iResolution.y*sin(timeScale*iGlobalTime);
//	float stripeWidth = iResolution.y / (2.0*stripeCount);
//	
//	if(mod(gl_FragCoord.y, 2.0*stripeWidth) > stripeWidth)
//	{
//		gl_FragColor = color1;
//	}
//	else
//	{
//		gl_FragColor = color2;
//	}
//}