uniform sampler2D tex;
uniform float pixelWidth;
uniform float pixelHeight;
uniform float imageWidth;
uniform float imageHeight;

void main(void)
{
    vec2 uv = gl_TexCoord[0].xy;
    float dx = pixelWidth*(1./imageWidth);
    float dy = pixelHeight*(1./imageHeight);
    vec2 coord = vec2(dx*floor(uv.x/dx),
                      dy*floor(uv.y/dy));
    gl_FragColor = texture2D(tex, coord);
}
