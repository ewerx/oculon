#extension GL_ARB_draw_instanced	: enable
#extension GL_EXT_gpu_shader4		: enable // Uncomment if ARB doesn't work
//#extension GL_EXT_draw_instanced	: enable // Uncomment if ARB doesn't work 

uniform vec3 size;
uniform vec3 spacing;

varying vec2 uv;
varying vec3 normal;
varying vec4 position;

void main()
{
	normal			= gl_Normal;
	position		= gl_Vertex;
	uv				= gl_MultiTexCoord0.st;
    
    float xysize    = size.x * size.y;
    float xy        = float( mod( float( gl_InstanceID ), xysize ) );
    float z         = float( floor( float( gl_InstanceID ) / xysize ) ) * -spacing.z;

	float x			= float( mod( xy, size.x ) ) * spacing.x;
    float y			= float( floor( xy / size.y ) ) * -spacing.y;
	x				-= spacing.x * size.x * 0.5;
    y               += spacing.y * size.y * 0.5;
	z				+= spacing.z * size.z * 0.5;
	vec3 offset		= vec3( x, y, z );
	position.xyz	+= offset;

	gl_Position		= gl_ModelViewProjectionMatrix * position;
}
