#version 120
uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/4tfGzj#
#define time iGlobalTime*.05

#define width .52

float thick=0.;
float pix;
float zoom = .5;


float formula(vec2 z) {
    float ot=1000.;
    for (int i=0; i<11; i++) {
        float dz=dot(z,z);
        z=abs(z*2.)/dz-1.;
        ot=min(ot,dz);
    }
    float h=.014/(max(0.,width-ot)/width*.9+.1);
    thick+=max(0.,1.-h);
    return h;
}

vec3 normal(vec2 z) {
    vec2 d=vec2(pix,0.);
    vec3 n=normalize(cross( //get normal
                           vec3(d.x*2.,0.,formula(z-d.xy)-formula(z+d.xy)),
                           vec3(0.,d.x*2.,formula(z-d.yx)-formula(z+d.yx))));
    return n;
}


void main()
{
    vec2 pos = gl_FragCoord.xy / iResolution.xy;
    vec2 uv=pos-.5;
    uv.x*=iResolution.x/iResolution.y;
    zoom*=1.+sin(time*2.)*.5;
    vec2 luv=uv;
    uv+=vec2(sin(time),cos(time))*2.;
    uv+=sin(uv*30.+time*200.)*.0015;
    uv*=zoom;
    luv*=zoom;
    pix=1./iResolution.x*zoom;
    vec2 d=vec2(pix,0.);
    vec3 n= normal(uv-d.xy)+normal(uv+d.xy);
    n+=normal(uv-d.yx)+normal(uv+d.yx);
    n/=4.;
    thick/=16.;
    vec2 refr=-n.xy*.08;
    vec3 tex=texture2D(iChannel0,((uv+refr)*3.+vec2(time,0.))).xyz+.75;
    vec3 colo=tex*mix(vec3(.35,.55,1.)*2.5,vec3(1.,.65,.15)*.25,sqrt(thick));
    vec3 lightdir=normalize(vec3(1.,.5,2.));
    colo+=max(0.,dot(-n,lightdir))*.4;
    colo+=pow(max(0.,dot(reflect(-n,vec3(0.,0.,-1.)),lightdir)),50.)*.6;
    colo+=pow(max(0.,dot(reflect(vec3(0.,0.,-1.),-n),lightdir)),50.)*.2;
    colo=pow(colo,vec3(1.5))*2.;
    gl_FragColor = vec4(colo,1.0);
}
