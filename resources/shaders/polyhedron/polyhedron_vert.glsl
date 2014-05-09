#extension GL_ARB_draw_instanced	: enable
#extension GL_EXT_gpu_shader4		: enable // Uncomment if ARB doesn't work
//#extension GL_EXT_draw_instanced	: enable // Uncomment if ARB doesn't work 

uniform vec3 size;
uniform vec3 spacing;

uniform sampler2D displacement;

varying vec2 uv;
varying vec3 normal;
varying vec4 position;

void main()
{
	normal			= gl_Normal;
	position		= gl_Vertex;
	uv				= gl_MultiTexCoord0.st;
    
    // instance coordinates
    float xysize    = size.x * size.y;
    float xy        = float( mod( float( gl_InstanceID ), xysize ) );
	float x			= float( mod( xy, size.x ) );
    float y			= float( floor( xy / size.y ) );
    float z         = float( floor( float( gl_InstanceID ) / xysize ) );
    
    // apply displacement on the vertices of the mesh
    //vec4 disp		= texture2D( displacement, uv );
    
    // apply displacement on the mesh instances
    vec4 disp       = texture2D( displacement, vec2(x,y) );
    
    x               *= spacing.x;
    y               *= -spacing.y;
    z               *= -spacing.z;
	x				+= spacing.x * size.x * 0.5;
    y               += spacing.y * size.y * 0.5;
	z				+= spacing.z * size.z * 0.5;
	vec3 offset		= vec3( x, y, z );
	position.xyz	+= offset * disp.x;

	gl_Position		= gl_ModelViewProjectionMatrix * position;
}
