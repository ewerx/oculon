varying vec4 texCoord;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    texCoord    = gl_MultiTexCoord0;
	gl_Position	= gl_ModelViewProjectionMatrix * gl_Vertex;
}
