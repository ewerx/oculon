uniform vec3      iResolution;     
uniform float     iGlobalTime;
uniform vec4      iColor1;
uniform vec4      iColor2;
uniform int iPoles;
uniform float iDistance;
uniform float iShift;

//
float ripple(float dist, float shift)
{
	return cos(64.0 * dist + shift) / (1.0 + 1.0 * dist);
}

void main(void)
{
	float larger = max(iResolution.x, iResolution.y);
	vec2 uv = (gl_FragCoord.xy - .5*iResolution.xy) / larger;
	vec2 uvflip = vec2(uv.x, -uv.y);
	
	float lum = .5;
	
	float twopi = 2.0*3.141592654;
	float fcount = float(iPoles);
	vec2 rot = vec2(cos(twopi*iGlobalTime*0.01), sin(twopi*iGlobalTime*0.01));
	vec2 tor = vec2(-sin(twopi*iGlobalTime*0.01), -cos(twopi*iGlobalTime*0.01));
    vec2 values = vec2( iDistance, iDistance );
	for (int i = 0; i < iPoles; ++i)
	{
		lum += .2 * ripple(length(values - uv), -iGlobalTime + iShift);
		values = values.x*rot + values.y*tor;
        rot = -rot;
        tor = -tor;
	}
	
    lum = 3.0*lum*lum - 2.0*lum*lum*lum;
	gl_FragColor = vec4(lum, lum, lum, 1.0);
    
    // vec2 blessr = vec2(-cursor.x, cursor.y);
    // float lum = .5 +
    //    .1 * ripple(length(cursor - uv), -iGlobalTime) +
    //    .1 * ripple(length(blessr - uv), -iGlobalTime) +
    //    .1 * ripple(length(cursor - uvflip), -iGlobalTime) +
    //    .1 * ripple(length(blessr - uvflip), -iGlobalTime) +
    //    .1 * ripple(length(uv), 0.0) +
    //    .1 * cos(64.0*uv.y - iGlobalTime) +
    //    .1 * cos(64.0*(uv.x*uv.x) - iGlobalTime) +
    //    0.0;
    
	/*float lum = .5, dist;
     vec2 part, flip = vec2(1.0, 1.0);
     
     //float freq = 64.0, phase = -iGlobalTime;
     float freq = 32.0, phase  = 0.0; // * pow(4.0, cos(iGlobalTime/8.0)), phase = 0.0;
     
     for (float ox = -REFLECTIONS; ox <= REFLECTIONS; ox += 1.0)
     {
     for (float oy = -REFLECTIONS; oy <= REFLECTIONS; oy += 1.0)
     {
     dist = length((cursor*flip-uv)+vec2(ox, oy));
     lum += cos(freq * dist - phase) / (5.0 + 10.0*dist);
     
     flip.y *= -1.0;
     }
     flip.x *= -1.0;
     }
     
	
	
	
	/*gl_FragColor = vec4(.5+.5*sin(3000.0*iGlobalTime),
     .5+.5*sin(4997.0*iGlobalTime+iResolution.x*3910.0),
     .5+.5*cos(2872.0*iGlobalTime+iResolution.y*8721.0), 1.0);*/
}

// colourised variation on "SoundSelf: Cymati.." by Cellulose, https://www.shadertoy.com/view/4dfGDs

//#define POLES 250.0
//#define SCALE 1.0
//
//#define REFLECTIONS 10.0

//void main(void)
//{
//	float larger = max(iResolution.x, iResolution.y) / SCALE;
//	vec2 uv = (gl_FragCoord.xy - .5*iResolution.xy) / larger;
//	vec2 cursor = (iMouse.xy - .5*iResolution.xy) / larger;
//	
//	float lum = .5;
//	float mul = lum;
//	float pi = 3.141592654;
//	float twopi = 2.0 * pi;
//	const float count = POLES;
//	float fcount = float(count);
//	vec2 rot = vec2(cos(twopi*.618),  sin(twopi*.618));
//	vec2 tor = vec2(-sin(twopi*.618), cos(twopi*.618));
//	float f = (0.7 + (1.0 + sin(mod(iGlobalTime/18.0, twopi))/4.0))/2.0;
//	vec3 col = vec3(0.0, 0.0, 0.0);
//    
//	for (float i = 0.0; i < count; ++i)
//	{
//		lum += .2 * ripple(length(cursor - uv), -iGlobalTime);
//		mul += .2 * ripple(length(cursor + uv * lum/16.0), -iGlobalTime/3.0);
//		col += vec3(mod(i + 1.0, 3.0), 0.0, 0.0) * 0.06 * (mul - lum);
//		col += vec3(0.0, mod(i + 2.0, 3.0), 0.0) * 0.06 * (lum - mul);
//		col += vec3(0.0, 0.0, mod(i, 3.0)) * 0.06 * sqrt(mul * lum);
//		cursor = cursor.x*rot*f + cursor.y*tor*f ;
//	}
//	gl_FragColor = vec4(col, 1.0);
//}

