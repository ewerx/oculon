uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

varying vec4 texCoord;

void main(void)
{

      // Sum colors from all textures
      vec4 color = texture2D(tex0, texCoord.st);
      color += texture2D(tex1, texCoord.st);
      color += texture2D(tex2, texCoord.st);
      color += texture2D(tex3, texCoord.st);

      // Set average
      gl_FragColor = color * 0.25;

}
