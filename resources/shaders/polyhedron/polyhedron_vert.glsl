#extension GL_ARB_draw_instanced	: enable
#extension GL_EXT_gpu_shader4		: enable // Uncomment if ARB doesn't work
//#extension GL_EXT_draw_instanced	: enable // Uncomment if ARB doesn't work 

uniform vec3 size;
uniform vec3 spacing;

uniform sampler2D displacement;
uniform sampler2D audioData;

varying vec2 uv;
varying vec3 normal;
varying vec4 position;
varying float instanceID;

mat3 rotationMatrix3(vec3 v, float angle)
{
	float c = cos(radians(angle));
	float s = sin(radians(angle));
	
	return mat3(c + (1.0 - c) * v.x * v.x, (1.0 - c) * v.x * v.y - s * v.z, (1.0 - c) * v.x * v.z + s * v.y,
                (1.0 - c) * v.x * v.y + s * v.z, c + (1.0 - c) * v.y * v.y, (1.0 - c) * v.y * v.z - s * v.x,
                (1.0 - c) * v.x * v.z - s * v.y, (1.0 - c) * v.y * v.z + s * v.x, c + (1.0 - c) * v.z * v.z
                );
}

void main()
{
	normal			= gl_Normal;
	position		= gl_Vertex;
	uv				= gl_MultiTexCoord0.st;
    instanceID      = float( gl_InstanceID );
    
    // instance coordinates
    float numInstances = size.x * size.y * size.z;
    
    
    
    float xysize    = size.x * size.y;
    float xy        = float( mod( float( instanceID ), xysize ) );
	float x			= float( mod( xy, size.x ) );
    float y			= float( floor( xy / size.y ) );
    float z         = float( floor( float( instanceID ) / xysize ) );
    
    // apply displacement on the vertices of the mesh
    vec4 disp		= texture2D( displacement, vec2(x,z) );
    
    float angle     = texture2D( audioData, vec2((x*y*z)/numInstances,0.0) ).b * 10.0;
    
    // apply displacement on the mesh instances
    //vec4 disp       = texture2D( displacement, vec2(x,y) );
    x = (x / size.x) - 0.5;
     y = (y / size.y) - 0.5;
     z = (z / size.z) - 0.5;
    
    
    
    x               *= spacing.x;
    y               *= spacing.y;
    z               *= spacing.z;
    
    
    
	vec3 offset		= vec3( x, y, z );
    
    
    position.xyz    *= rotationMatrix3( disp.xyz, angle );
    
    position.xyz    += offset;
	//position.xyz	+= disp.xyz * spacing.x;

	gl_Position		= gl_ModelViewProjectionMatrix * position;
}
