uniform bool		lightingEnabled;
uniform vec3		eyePoint;
//uniform sampler2D	tex;
uniform bool		textureEnabled;
uniform bool        flat;

varying vec3		normal;
varying vec4		position;
varying vec2		uv;

uniform sampler2DShadow	tex0;	// shadow depth texture

varying vec3 V;
varying vec3 N;
varying vec4 Q;

void main( void )
{
	vec4 color			= vec4( 0.0, 0.0, 0.0, 0.0 );
    
/*
	if ( textureEnabled ) {
		color			= texture2D( tex, uv );
	}
	if ( lightingEnabled ) {
		float shininess	= 20.0;
		vec3 eye		= normalize( -eyePoint );
		vec3 light		= normalize( gl_LightSource[ 0 ].position.xyz - position.xyz );   
		vec3 reflection = normalize( -reflect( light, normal ) );

		vec4 ambient	= gl_LightSource[ 0 ].ambient;
		vec4 diffuse	= clamp( gl_LightSource[ 0 ].diffuse * max( dot( normal.xyz, light ), 0.0 ), 0.0, 1.0 );     
		vec4 specular	= clamp( gl_LightSource[ 0 ].specular * pow( max( dot( reflection, eye ), 0.0 ), 0.3 * shininess ), 0.0, 1.0 ); 

		color			+= ambient + diffuse + specular;
	} else {
		color			= vec4( 1.0, 1.0, 1.0, 1.0 );
	}

	gl_FragColor	= color;
*/
    
    // houx shadow/lightning
    if ( lightingEnabled ) 
    {
        const vec4  gamma = vec4(1.0 / 2.2); 
        const float shininess = 500.0;

        vec4 ambient = gl_LightSource[0].ambient;
        vec4 diffuse = gl_LightSource[0].diffuse;
        vec4 specular = gl_LightSource[0].specular;

        // if flat shading is enabled, calculate face normal on-the-fly
        vec3 n = flat ? normalize(cross(dFdx(V), dFdy(V))) : N;

        vec3 L = normalize(gl_LightSource[0].position.xyz - V);
        vec3 E = normalize(-V);
        vec3 R = normalize(-reflect(L,n));
        
        // shadow term
        float shadow = 0.1 + 0.9 * shadow2D( tex0, 0.5 * (Q.xyz / Q.w + 1.0) ).r;

        // ambient term 
        vec4 Iamb = ambient;

        // diffuse term
        vec4 Idiff = vec4( 1.0, 1.0, 1.0, 1.0 ) * diffuse;//texture2D( tex1, gl_TexCoord[0].st) * diffuse;
        Idiff *= max(dot(n,L), 0.0);
        Idiff = clamp(Idiff, 0.0, 1.0) * shadow;

        // specular term
        vec4 Ispec = specular;
        Ispec *= pow(max(dot(R,E),0.0), shininess);
        Ispec = clamp(Ispec, 0.0, 1.0) * shadow;

        // final color 
        color = pow(Iamb + Idiff + Ispec, gamma);
    }
    else 
    {
        color			= vec4( 1.0, 1.0, 1.0, 1.0 );
	}

    gl_FragColor	= color;
}
