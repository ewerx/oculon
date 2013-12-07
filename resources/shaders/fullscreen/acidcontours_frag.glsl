uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform sampler2D iChannel0;
uniform vec2      iMouse;

float dist(vec2 p1, vec2 p2) {
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	return sqrt(dx * dx + dy * dy);	
}

float dotp(vec2 p1, vec2 p2) {
	return p1.x * p2.x + p1.y * p2.y;	
}

float getR(float h) {
	return cos(h*6.283185307179586476925286766559)*0.5+0.5;
}

float getG(float h) {
	return cos((h+1.0/3.0)*6.283185307179586476925286766559)*0.5+0.5;
}

float getB(float h) {
	return cos((h+2.0/3.0)*6.283185307179586476925286766559)*0.5+0.5;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float t = iGlobalTime;
	vec2 a = vec2(0.5,0.5);
	vec2 b = vec2(0.25,0.625);
	vec2 c = vec2(0.75,0.625);
	
	float d = cos(dist(uv,a)*17.0)+ cos(dist(uv,b)*15.0+t*0.2) + cos(dist(uv,c)*13.0+t*0.3);

	gl_FragColor = vec4(getR(d), getG(d), getB(d), 1.0);
}