uniform float amountX;
uniform float amountY;
uniform sampler2D tex0;
uniform float blurCenterWeight;

void main() {
	vec4 col = texture2D(tex0, gl_TexCoord[0].xy);
	float aX = amountX * (col.r + 0.5);
	float aY = amountY * (col.r + 0.5);
	
	col *= 4.0 + blurCenterWeight;
	col += 1.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(-aX, -aY));	
	col += 2.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(0.0, -aY));
	col += 1.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(aX, -aY));
	col += 2.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(aX, 0.0));
	col += 1.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(aX, aY));
	col += 2.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(0.0, aY));
	col += 1.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(-aX, aY));	
	col += 2.0 * texture2D(tex0, gl_TexCoord[0].xy + vec2(-aX, 0.0));	
	gl_FragColor = col/(16.0 + blurCenterWeight);
}
