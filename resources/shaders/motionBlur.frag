uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform bool pixelate;
uniform float pixelWidth;
uniform float pixelHeight;
uniform float imageWidth;
uniform float imageHeight;

varying vec4 texCoord;

void main(void)
{
    vec2 uv = texCoord.st;
    vec2 coord = uv;
    if (pixelate) {
        float dx = pixelWidth*(1./imageWidth);
        float dy = pixelHeight*(1./imageHeight);
        coord = vec2(dx*floor(uv.x/dx), dy*floor(uv.y/dy));
    }
    
    // Sum colors from all textures
    vec4 color = texture2D(tex0, coord);
    color += texture2D(tex1, coord);
    color += texture2D(tex2, coord);
    color += texture2D(tex3, coord);
    
    // Set average
    gl_FragColor = color * 0.25;
}
