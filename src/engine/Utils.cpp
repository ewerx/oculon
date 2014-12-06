//
//  Utils.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-02-08.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "Utils.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma mark - path helpers

/*static*/ ci::fs::path Utils::getUniquePath( const std::string &path, int padding, const std::string &sep ) 
{
    // expand to resolve ~
	fs::path p = expandPath( fs::path( path ) ); 
	string extension = p.extension().string();
	string stem = p.stem().string();
	fs::path parent_path = p.parent_path();
    
	int count = 0;
    
	do
	{
		p = parent_path / ( stem + sep + leftPaddedString( toString(count), padding ) + extension );
		++count;
	}
    while ( fs::exists( p ) && count < math<int>::pow(10, padding) );
    
	return p;
}

/*static*/ std::string Utils::leftPaddedString( const std::string &input, int padding, const std::string pad )
{
	std::string output(input);
	while ( output.size() < padding )
	{
		output = pad + output;
	}
    
	return output;
}

#pragma mark - conversions
 
/*static*/ Vec2f Utils::toMercatorProjection( const float aLat, const float aLong, const float aMapWidth, const float aMapHeight ) 
{
    // mercator projection center was screen centered
    Vec2f offset = Vec2f( aMapWidth / 2, aMapHeight / 2 );
    /* we need to place all the earth loft exactly at window area */
    float radius = aMapWidth / ( 2 * M_PI );

    float x = radius * aLong * M_PI / 180 + offset.x;
    
    /* here is the black magick */
    float y = offset.y - radius * ( log( tan( ( 90 + aLat ) * M_PI / 360 ) )  /  M_PI ) * 2.8f;
    
    return Vec2f( x, y );
}

/*static*/ Vec2f Utils::toEquirectProjection( const float aLat, const float aLong, const float aMapWidth, const float aMapHeight, const int aMapOffset ) 
{
    float offsetLong = aLong + 180 + aMapOffset;
    offsetLong = fmod(offsetLong, 360.0f);
    float x = offsetLong * (aMapWidth / 360);
    float y = (90 - aLat) * (aMapHeight / 180);
    if( x < 0.0f || y < 0.0f || x > aMapWidth || y > aMapHeight )
    {
        console() << "lat/long " << aLat << ", " << aLong << " --> " << x << ", " << y << std::endl;
    }
    
    return Vec2f( x, y );
}

/*static*/ Vec3f Utils::toGlobePosition( const float aLat, const float aLong )
{
    float theta = toRadians( 90 - aLat );
    float phi	= toRadians( 180 - aLong );
    
	return Vec3f( sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi) );
}

/*static*/ time_t Utils::toEpochSeconds(boost::posix_time::ptime& t)
{
    using namespace boost::posix_time;
    //ptime epoch(boost::gregorian::date(1970,1,1));
    //time_duration::sec_type x = (t - epoch).total_seconds();
    
    //TODO: check overflow
    
    //return time_t(x);
    return 0;
}

/*static*/ Color Utils::toColor(uint32_t hex)
{
	float r = ((hex & 0x00FF0000) >> 16) / 255.0f;
	float g = ((hex & 0x0000FF00) >> 8) / 255.0f;
	float b = ((hex & 0x000000FF)) / 255.0f;
    
	return Color(r, g, b);
}

/*static*/ ColorA Utils::toColorA(uint32_t hex)
{
	float a = ((hex & 0xFF000000) >> 24) / 255.0f;
	float r = ((hex & 0x00FF0000) >> 16) / 255.0f;
	float g = ((hex & 0x0000FF00) >> 8) / 255.0f;
	float b = ((hex & 0x000000FF)) / 255.0f;
    
	return ColorA(r, g, b, a);
}

/*static*/ int Utils::toInt(const std::string &str)
{
	int x;
	std::istringstream i(str);
    
	if (!(i >> x)) throw std::exception();
    
	return x;
}

/*static*/ double Utils::toDouble(const std::string &str)
{
	double x;
	std::istringstream i(str);
    
	if (!(i >> x)) throw std::exception();
    
	return x;
}

#pragma mark - shader helpers

gl::GlslProg Utils::loadFragShader( const std::string& filename )
{
    return loadVertAndFragShaders("passThru.vert", filename);
}

gl::GlslProg Utils::loadVertAndFragShaders(const std::string &vertShader, const std::string &fragShader)
{
    gl::GlslProg shader;
    
    try
    {
        shader = gl::GlslProg( loadResource( vertShader ), loadResource( fragShader ) );
    }
    catch( gl::GlslProgCompileExc &exc )
    {
		console() << "Shader (" << vertShader << " / " << fragShader << ") compile error: " << std::endl;
		console() << exc.what();
        assert(false);
	}
	catch( ... )
    {
		console() << "Unable to load shaders (" << vertShader << " / " << fragShader << ")"  << std::endl;
        assert(false);
	}
    
    return shader;
}

void Utils::drawTexturedRect(const ci::Area &bounds)
{
    // Draw shader output
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::begin( GL_TRIANGLES );
    
    Vec2f vert0( (float)bounds.x1, (float)bounds.y1 );
    Vec2f vert1( (float)bounds.x2, (float)bounds.y1 );
    Vec2f vert2( (float)bounds.x1, (float)bounds.y2 );
    Vec2f vert3( (float)bounds.x2, (float)bounds.y2 );
    
    // Define quad texture coordinates
    Vec2f uv0( 0.0f, 0.0f );
    Vec2f uv1( 1.0f, 0.0f );
    Vec2f uv2( 0.0f, 1.0f );
    Vec2f uv3( 1.0f, 1.0f );
    
    // Draw quad (two triangles)
    gl::texCoord( uv0 );
    gl::vertex( vert0 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv3 );
    gl::vertex( vert3 );
    
    gl::end();
}
