uniform vec3      iResolution;     // viewport resolution (in pixels)
uniform float     iGlobalTime;     // shader playback time (in seconds)
uniform vec3      borderColor;
uniform float     zoom;
uniform float     speed;
uniform float     borderIn;
uniform float     borderOut;
uniform float     seedSize;
uniform vec3      seedColor;
uniform float     cellLayers;
uniform float     cellBorderStrength;
uniform float     cellBrightness;
uniform vec3      cellColor;
uniform float     distortion;
//uniform sampler2D iChannel0;

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// I've not seen anybody out there computing correct cell interior distances for Voronoi
// patterns yet. That's why they cannot shade the cell interior correctly, and why you've
// never seen cell boundaries rendered correctly. 
//
// However, here's how you do mathematically correct distances (note the equidistant and non
// degenerated grey isolines inside the cells) and hence edges (in yellow):
//
// http://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm


#define ANIMATE

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

vec2 hash( vec2 p )
{
    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return fract(sin(p)*43758.5453);
}

vec3 voronoi( in vec2 x )
{
    vec2 n = floor(x);
    vec2 f = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mg, mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
		vec2 o = hash( n + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*distortion*sin( iGlobalTime*speed + 6.2831*o );
        #endif	
        vec2 r = g + o - f;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
		vec2 o = hash( n + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*distortion*sin( iGlobalTime*speed + 6.2831*o );
        #endif	
        vec2 r = g + o - f;

        if( length(mr-r)<0.0001 ) continue;

        // distance to line		
        float d = dot( 0.5*(mr+r), normalize(r-mr) );

        md = min( md, d );
    }

    return vec3( md, mr );
}

void main( void )
{
    vec2 p = gl_FragCoord.xy/iResolution.xx;

    vec3 c = voronoi( zoom*p );

	// isolines
    vec3 col = c.x*(cellBrightness + cellBorderStrength*sin(8.0*cellLayers*c.x))*cellColor;
    // borders	
    col = mix( borderColor, col, smoothstep( borderIn, borderOut, c.x ) );
    // feature points
	float dd = length( c.yz );
    col = mix( seedColor, col, smoothstep( 0.0, seedSize, dd) );
    col += seedColor*(1.0-smoothstep( 0.0, seedSize/3.0, dd));

	gl_FragColor = vec4(col,1.0);
}

