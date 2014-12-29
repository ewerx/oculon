/*
 Copyright (c) 2010-2012, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "ParsecBackground.h"
#include "Conversions.h"

#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// define constants used in the coordinate conversion methods
const Vec3d	ParsecBackground::GALACTIC_CENTER_EQUATORIAL = Vec3d(toRadians(266.40510), toRadians(-28.936175), 8.33);
const Vec2d	ParsecBackground::GALACTIC_NORTHPOLE_EQUATORIAL = Vec2d(toRadians(192.859508), toRadians(27.128336));

ParsecBackground::ParsecBackground(void)
	: mAttenuation(1.0f)
    , mAlpha(1.0f)
{
	// Transform the map from galactic coordinates to equatorial coordinates. 
	// OpenGL matrix derived from http://arxiv.org/pdf/1010.3773.pdf
	mTransform = Matrix44f( +0.444829594298f, -0.746982248696f, -0.494109453633f, 0.0f,
							-0.198076389622f, +0.455983794523f, -0.867666135681f, 0.0f,
							+0.873437104725f, +0.483834991775f, +0.054875539390f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f );
}

ParsecBackground::~ParsecBackground(void)
{
}

void ParsecBackground::setup()
{
	mTexture = gl::Texture( loadImage( loadResource("parsec-background.jpg") ) );

	create();
}

void ParsecBackground::draw()
{
	if(!( mTexture && mVboMesh )) return;

	glPushAttrib( GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT );

	mTexture.enableAndBind();

	gl::pushModelView();
	gl::multModelView( mTransform );
	gl::color( mAttenuation * mAlpha * Color::white() );
	gl::draw( mVboMesh );
	gl::popModelView();

	glPopAttrib();
}


void ParsecBackground::create()
{
	const double	TWO_PI = 2.0 * M_PI;
	const double	HALF_PI = 0.5 * M_PI;

	const int		SLICES = 30;
	const int		SEGMENTS = 60;
	const int		RADIUS = 2000;

	// create data buffers
	vector<Vec3f>		normals;
	vector<Vec3f>		positions;
	vector<Vec2f>		texCoords;
	vector<uint32_t>	indices;

	//	
	int x, y;
	for(x=0;x<=SEGMENTS;++x) {
		double theta = static_cast<double>(x) / SEGMENTS * TWO_PI;

		for(y=0;y<=SLICES;++y) {
			double phi = (0.5 - static_cast<double>(y) / SLICES) * M_PI;

			normals.push_back( Vec3f(
				static_cast<float>( cos(phi) * sin(theta) ),
				static_cast<float>( sin(phi) ),
				static_cast<float>( cos(phi) * cos(theta) ) ) );

			positions.push_back( normals.back() * RADIUS );	

			float tx = 1.0f - static_cast<float>(x) / SEGMENTS;
			float ty = static_cast<float>(y) / SLICES;

			texCoords.push_back( Vec2f(tx, ty) );
		}
	}

	//
	int rings = SLICES+1;
	bool forward = false;
	for(x=0;x<SEGMENTS;++x) {
        if(forward) {
			// create jumps in the triangle strip by introducing degenerate polygons
			indices.push_back(  x      * rings + 0 );
			indices.push_back(  x      * rings + 0 );
			// 
            for(y=0;y<rings;++y) {
                indices.push_back(  x      * rings + y );
                indices.push_back( (x + 1) * rings + y );
            }
        }
        else {
			// create jumps in the triangle strip by introducing degenerate polygons
			indices.push_back( (x + 1) * rings + SLICES );
			indices.push_back( (x + 1) * rings + SLICES );
			// 
            for(y=SLICES;y>=0;--y) {
                indices.push_back( (x + 1) * rings + y );
                indices.push_back(  x      * rings + y );
            }
        }

        forward = !forward;
    }

	// create the mesh
	gl::VboMesh::Layout layout;	
	layout.setStaticIndices();
	layout.setStaticPositions();
	layout.setStaticTexCoords2d();
	layout.setStaticNormals();

	mVboMesh = gl::VboMesh(positions.size(), indices.size(), layout, GL_TRIANGLE_STRIP);

	mVboMesh.bufferNormals( normals );
	mVboMesh.bufferTexCoords2d( 0, texCoords );
	mVboMesh.bufferPositions( positions );
	mVboMesh.bufferIndices( indices );
    mVboMesh.unbindBuffers();
}

void ParsecBackground::setCameraDistance( float distance )
{
	static const float minimum = 0.5f;
	static const float maximum = 1.0f;

	if( distance > 300.0f ) {
		mAttenuation = ci::lerp<float>( minimum, 0.0f, (distance - 300.0f) / 200.0f );
		mAttenuation = math<float>::clamp( mAttenuation, 0.0f, maximum );
	}
	else {
		mAttenuation = math<float>::clamp( 1.0f - math<float>::log10( distance ) / math<float>::log10(100.0f), minimum, maximum );
	}
}

Vec2d ParsecBackground::toEquatorial( const Vec2d &radians )
{	
	double longitude = radians.x;	// galactic longitude
	double latitude = radians.y;	// galactic latitude
	
	double alpha = GALACTIC_NORTHPOLE_EQUATORIAL.x;
	double delta = GALACTIC_NORTHPOLE_EQUATORIAL.y;
	double la = toRadians(33.0);

	double dec = asin(sin(latitude) * sin(delta) + cos(latitude) * cos(delta) * sin(longitude - la));
	double ra = atan2(cos(latitude) * cos(longitude - la), sin(latitude) * cos(delta) - cos(latitude) * sin(delta) * sin(longitude - la)) + alpha;
	
	ra = Conversions::wrap(ra, 0.0, 2.0 * M_PI);

	return Vec2d(ra, dec);
}

Vec2d ParsecBackground::toGalactic( const Vec2d &radians )
{
	double ra = radians.x;
	double dec = radians.y;

	double alpha = GALACTIC_NORTHPOLE_EQUATORIAL.x;
	double delta = GALACTIC_NORTHPOLE_EQUATORIAL.y;
	double la = toRadians(33.0);
    
    double latitude = asin(sin(dec) * sin(delta) + cos(dec) * cos(delta) * cos(ra - alpha));
    double longitude = atan2(sin(dec) * cos(delta) - cos(dec) * sin(delta) * cos(ra - alpha), cos(dec) * sin(ra - alpha)) + la;

	longitude = Conversions::wrap(longitude, 0.0, 2.0 * M_PI);

	return Vec2d(longitude, latitude);
}
