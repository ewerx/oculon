uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;
uniform float     iWobbleAmount;
uniform float     iWobbleRate;

// https://www.shadertoy.com/view/4l23Rw

float compute(vec2 p, float i){
    float a = atan(p.y,p.x);
    float l = length(p);
    float d = mod(l-i*.5,1.); // fire rate
    d-=.5;
    d = abs(d);
    d-=sin(d)*.29+.1; // light cycle
    d = abs(d);
    d/=sin(a*2.+sin(l*l+i*.3)*6.)+1.2; // twists + spin + darkness
    return 1.0-smoothstep(0.,.05, d/max(.215,p.y)); // glow
}

void main()
{
    float i=  iGlobalTime;
    vec4 c = vec4(1.0);
    vec2 uv = gl_FragCoord.xy / iResolution.xx*2.-1.;
    uv.y += 1.;
    
    float l = length(uv);
    float a = atan(uv.y,uv.x);
    float deg = sin(i)*2.+0.8; // max folds + min folds
    float j = sin(i)*.3; // stretch
    float d = sin(i*1.1415)*.03; // wobble rate * amount
    uv*=mat2(sin(a*deg+j-deg)*l,cos(a*deg-deg)*l,-cos(a*deg+j-deg)*l,sin(a*deg-deg)*l);
    
    float w;
    for (float ii = 0.0; ii < 5.0; ii++) {
        w += (1.0/3.0)*compute(uv,i);
        uv.y+=d;
    }
    
    c = vec4(w,w,w,1.0);
//    c.r = compute(uv,i);
//    uv.y-=d;
//    c.g = compute(uv,i);
//    //uv.y+=d*2.;
//    c.b = compute(uv,i);
    
    gl_FragColor = c;
}
