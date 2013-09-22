uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform float timeScale;
uniform float countScale;
uniform vec4 color1;
uniform vec4 color2;

void main(void)
{
	float stripeCount = countScale*10.0 + countScale*iResolution.y*sin(timeScale*iGlobalTime);
	float stripeWidth = iResolution.y / (2.0*stripeCount);
	
	if(mod(gl_FragCoord.y, 2.0*stripeWidth) > stripeWidth)
	{
		gl_FragColor = color1;
	}
	else
	{
		gl_FragColor = color2;
	}
}