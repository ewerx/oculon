uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/4l23Rw

float compute(vec2 p, float i){
    float a = atan(p.y,p.x);
    float l = length(p);
    float d = mod(l-i*.5,1.);
    d-=.5;
    d = abs(d);
    d-=sin(d)*.05+.1;
    d = abs(d);
    d/=sin(a*12.+sin(l*l+i*.3)*10.)+2.;
    return 1.0-smoothstep(0.,.05, d/max(.15,p.y));
}

void main()
{
    float i=  iGlobalTime*.75;
    vec4 c = vec4(1.0);
    vec2 uv = gl_FragCoord.xy / iResolution.xx*2.-1.;
    uv.y += 1.;
    
    float l = length(uv);
    float a = atan(uv.y,uv.x);
    float deg = sin(i)*2.+8.;
    float j = sin(i)*.3;
    float d = sin(i*3.1415)*.03;
    uv*=mat2(sin(a*deg+j-deg)*l,cos(a*deg-deg)*l,-cos(a*deg+j-deg)*l,sin(a*deg-deg)*l);
    
    c.r = compute(uv,i);
    uv.y-=d;
    c.g = compute(uv,i);
    uv.y+=d*2.;
    c.b = compute(uv,i);
    
    gl_FragColor = c;
}