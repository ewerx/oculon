//
//  PlanetObject.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-26.
//
//

#include "PlanetObject.h"
#include "Utils.h"
#include "MeshHelper.h"

using namespace ci;
using namespace ci::app;
using namespace std;


PlanetObject::PlanetObject()
: mPosition(Vec3f::zero())
, mRadius(25.0f)
{
    mShader = Utils::loadFragShader("earth_frag.glsl");
    
    mTexDiffuse = gl::Texture( loadImage( loadResource( "moon_diffuse.jpg" ) ) );
    mTexMask = gl::Texture( loadImage( loadResource( "moon_hypsometric.jpg" ) ) );
    mTexNormal = gl::Texture( loadImage( loadResource( "moon_normalmap.jpg" ) ) );
    
    mLightDir		= Vec3f( 0.025f, 0.25f, 1.0f );
	mLightDir.normalize();
    
    mMesh = gl::VboMesh( MeshHelper::createSphere( Vec2i(24, 12) ) );
}

PlanetObject::~PlanetObject()
{
}

void PlanetObject::draw()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    
    gl::enableAlphaBlending();
	gl::enableDepthRead( true );
	gl::enableDepthWrite( true );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_RECTANGLE_ARB );
    
    mTexDiffuse.bind( 0 );
	mTexNormal.bind( 1 );
	mTexMask.bind( 2 );
    
    mShader.bind();
    mShader.uniform( "texDiffuse", 0 );
    mShader.uniform( "texNormal", 1 );
    mShader.uniform( "texMask", 2 );
    mShader.uniform( "lightDir", mLightDir );
    
    //gl::draw( mMesh );
    gl::drawSphere( mPosition, mRadius, 64 );
    
    mShader.unbind();
    mTexNormal.unbind();
    mTexMask.unbind();
    mTexDiffuse.unbind();
    
    glPopAttrib();
}

