#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/4tfGzj#
//parameters
const int iterations=27;
const float scale=1.3;
const vec2 fold=vec2(.5);
const vec2 translate=vec2(1.5);
const float zoom=.25;
const float brightness=7.;
const float saturation=.65;
const float texturescale=.15;
const float rotspeed=.001;
const float colspeed=.005;
const float antialias=2.;


vec2 rotate(vec2 p, float angle) {
    return vec2(p.x*cos(angle)-p.y*sin(angle),
                p.y*cos(angle)+p.x*sin(angle));
}

void main()
{
    vec3 aacolor=vec3(0.);
    vec2 pos=gl_FragCoord.xy / iResolution.xy-.5;
    float aspect=iResolution.y/iResolution.x;
    pos.y*=aspect;
    pos/=zoom;
    vec2 pixsize=max(1./zoom,100.-iGlobalTime*50.)/iResolution.xy;
    pixsize.y*=aspect;
    for (float aa=0.; aa<25.; aa++) {
        if (aa+1.>antialias*antialias) break;
        vec2 aacoord=floor(vec2(aa/antialias,mod(aa,antialias)));
        vec2 p=pos+aacoord*pixsize/antialias;
        p+=fold;
        float expsmooth=0.;
        vec2 average=vec2(0.);
        float l=length(p);
        for (int i=0; i<iterations; i++) {
            p=abs(p-fold)+fold-iMouse.y/99.;
            p=p*scale-translate;
            if (length(p)>20.) break;
            p=rotate(p,iGlobalTime*rotspeed+iMouse.x/77.+27.5);
            average+=p;
        }
        average/=float(iterations);
        vec2 coord=average+vec2(iGlobalTime*colspeed+33.);
        vec3 color=texture2D(iChannel0,coord*texturescale).xyz;
        color*=min(1.1,length(average)*brightness);
        color=mix(vec3(length(color)),color,saturation);
        aacolor+=color;
    }
    gl_FragColor = vec4(aacolor/(antialias*antialias),1.0);
}