#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;
uniform float     iSmoothness;
uniform float     iLength;

/*by musk License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 
	Audio Visualizaer using my graphing tool. https://www.shadertoy.com/view/4dS3WG
 
 */

#define quality 60
#define smoothness iSmoothness
//#define grid
//#define subpixel
//#define reverseLCD

float f(float x)
{
    float q = .0;
    
    if (x<0.0)
    {
        x = x/iLength+1.0;
        q = texture2D(iChannel1,vec2(x,1.0)).y-.666;
        q*= texture2D(iChannel1,vec2(0.0,0.0)).y*8.0;
    }
    else
    {
        x=x/iLength;
        x=pow(x*.82+.18,4.0);
        q = texture2D(iChannel1,vec2(x,0.0)).y-.333;
    }
    q*=4.0;
    return q;
}

float hash(float x)
{
    return fract(sin(x*164437.36831)*13217.321); //decent hash for noise generation
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy-.5;
    uv.x = uv.x*iResolution.x/iResolution.y;
    uv*=2.0;
    float pixel = 4.0/iResolution.y;
    
    uv*=4.0;
    
    vec3 c = vec3(.0);
    
    for (float fi=.0; fi<1.0; fi += 1.0/float(quality))
    {
#ifdef subpixel
        for (int i=0; i<3; i++)
        {
            float x = uv.x + hash(uv.y*41.0+fi)*pixel*float(smoothness)*.3333 + float(i)*pixel*.3333;
            float y = uv.y + hash(uv.x*41.0+fi)*pixel*float(smoothness);
            
            float s0 = f(x-pixel);
            float s1 = f(x+pixel);
            
            if (s0-pixel <= y && y < s1+pixel || s1-pixel <= y && y < s0+pixel)
            {
#ifdef reverseLCD
                c[2-i] += 1.0;
#else
                c[i] += 1.0;
#endif
            }
        }
#else
        float x = uv.x + hash(uv.y*41.0+fi)*pixel*float(smoothness);
        float y = uv.y + hash(uv.x*41.0+fi)*pixel*float(smoothness);
        
        float s0 = f(x-pixel);
        float s1 = f(x+pixel);
        
        if (s0-pixel <= y && y < s1+pixel || s1-pixel <= y && y < s0+pixel)
        {
            c += vec3(1.0);
        }
#endif
    }
    
    c/=float(quality);
    
#ifdef grid
    c += max(.0,1.0-abs(mod(uv.x+.5,1.0)-.5)/pixel*0.5)*.1/(.5+abs(uv.x)*.5);
    c += max(.0,1.0-abs(mod(uv.y+.5,1.0)-.5)/pixel*0.5)*.1/(.5+abs(uv.y)*.5);
#endif
    
    gl_FragColor = vec4(pow(c,vec3(.6)),1.0);
}
