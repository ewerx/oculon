uniform float		height;
uniform sampler2D	displacement;
uniform vec3		scale;
uniform mat4        shadowMatrix;
uniform bool		lightingEnabled;

varying vec3		normal;
varying vec4		position;
varying vec2		uv;

varying vec3 V;
varying vec3 N;
varying vec4 Q;

void main( void )
{
	normal			= gl_Normal;
	uv				= gl_MultiTexCoord0.st;
	vec4 color		= texture2D( displacement, uv );
	float offset	= color.r * height;
	position		= gl_Vertex * vec4( scale, 1.0 );
	position.xyz	+= normal * scale * offset;
	gl_Position		= gl_ModelViewProjectionMatrix * position;
    
    if( lightingEnabled ) {
        // transform vertex into eyespace
        V = (gl_ModelViewMatrix * gl_Vertex).xyz;
        
        // transform normal into eyespace
        N = normalize(gl_NormalMatrix * gl_Normal);
        
        // needed by the shadow map algorithm
        Q = shadowMatrix * gl_ModelViewMatrix * gl_Vertex;
    }
}
