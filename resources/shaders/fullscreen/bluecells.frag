uniform vec2      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback iGlobalTime (in seconds)
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform vec3      iMouse;

// https://www.shadertoy.com/view/llS3RK

//Calculate the squared length of a vector
float length2(vec2 p){
    return dot(p,p);
}

//Generate some noise to scatter points.
float noise(vec2 p){
    return fract(sin(fract(sin(p.x) * (43.13311)) + p.y) * 31.0011);
}

float worley(vec2 p) {
    //Set our distance to infinity
    float d = 1e30;
    //For the 9 surrounding grid points
    for (int xo = -1; xo <= 1; ++xo) {
        for (int yo = -1; yo <= 1; ++yo) {
            //Floor our vec2 and add an offset to create our point
            vec2 tp = floor(p) + vec2(xo, yo);
            //Calculate the minimum distance for this grid point
            //Mix in the noise value too!
            d = min(d, length2(p - tp - noise(tp)));
        }
    }
    return 3.0*exp(-4.0*abs(2.5*d - 1.0));
}

float fworley(vec2 p) {
    //Stack noise layers
    return sqrt(sqrt(sqrt(
                          worley(p*5.0 + 0.05*iGlobalTime) *
                          sqrt(worley(p * 50.0 + 0.12 + -0.1*iGlobalTime)) *
                          sqrt(sqrt(worley(p * -10.0 + 0.03*iGlobalTime))))));
}

// another worley: https://www.shadertoy.com/view/MlB3Rm

float r(float n)
{
    return fract(cos(n*89.42)*343.42);
}

vec2 r(vec2 n)
{
    return vec2(r(n.x*23.62-300.0+n.y*34.35),r(n.x*45.13+256.0+n.y*38.89));
}

float worley2(vec2 P,vec2 R,float s)
{
    vec2 n = P+iGlobalTime*s-0.5*R;
    float dis = 64.0;
    for(int x = -1;x<2;x++)
    {
        for(int y = -1;y<2;y++)
        {
            vec2 p = floor(n/s)+vec2(x,y);
            float d = length(r(p)+vec2(x,y)-fract(n/s));
            if (dis>d)
            {
                dis = d;
            }
        }
    }
    return pow(dis,4.0);
}
//

void main()
{
    bool w1 = false;
    
    if (w1) {
        vec2 uv = gl_FragCoord.xy / iResolution.xy;
        //Calculate an intensity
        float t = fworley(uv * iResolution.xy / 1500.0);
        //Add some gradient
        t*=exp(-length2(abs(0.7*uv - 1.0)));
        //Make it blue!
        gl_FragColor = vec4(t * vec3(0.1, 1.1*t, pow(t, 0.5-t)), 1.0);
    }
    else {
        vec2 c = gl_FragCoord.xy;
        vec2 r = iResolution.xy;
        vec3 col = vec3(worley2(c,r,32.0)+worley2(c,r,36.0)+worley2(c,r,44.0));
        gl_FragColor = vec4(col,1.0);
    }
    
    
}

