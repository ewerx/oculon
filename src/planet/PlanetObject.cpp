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

GLfloat PlanetObject::no_mat[]			= { 0.0f, 0.0, 0.0f, 1.0f };
GLfloat PlanetObject::mat_ambient[]		= { 0.5f, 0.5, 0.5f, 1.0f };
GLfloat PlanetObject::mat_diffuse[]		= { 0.8f, 0.8, 0.8f, 1.0f };
GLfloat PlanetObject::mat_specular[]	= { 1.0f, 1.0, 1.0f, 1.0f };
GLfloat PlanetObject::mat_emission[]	= { 0.15f, 0.15f, 0.15f, 0.0f };

GLfloat PlanetObject::mat_shininess[]	= { 128.0f };
GLfloat PlanetObject::no_shininess[]	= { 0.0f };


PlanetObject::PlanetObject()
: mPosition(Vec3f(50.0f,60.0f,30.0f))
, mRadius(25.0f)
{
    mShader = Utils::loadFragShader("earth_frag.glsl");
    
    gl::Texture::Format format;
	//format.setColorInternalFormat( GL_RGBA32F_ARB );
	format.setWrap( GL_REPEAT, GL_REPEAT );
    
    mTexDiffuse = gl::Texture( loadImage( loadResource( "moon_diffuse.jpg" ) ), format );
    mTexMask = gl::Texture( loadImage( loadResource( "moon_hypsometric.jpg" ) ), format );
    mTexNormal = gl::Texture( loadImage( loadResource( "moon_normalmap.jpg" ) ), format );
    
    mLightDir		= Vec3f( 0.025f, 0.25f, 1.0f );
	mLightDir.normalize();
    
    //mMesh = gl::VboMesh( MeshHelper::createSphere( Vec2i(24, 12) ) );
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
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_TEXTURE_RECTANGLE_ARB );
    
    // lighting
    {
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0 );
        
        GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightfv( GL_LIGHT0, GL_POSITION, light_position );
        glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.0f );
        glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f );
        glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.00015f );
        
        ColorA color( 0.5f, 0.5f, 0.5f, 1.0f );
        glMaterialfv( GL_FRONT, GL_DIFFUSE, color );
        glMaterialfv( GL_FRONT, GL_AMBIENT,	PlanetObject::no_mat );
        glMaterialfv( GL_FRONT, GL_SPECULAR, PlanetObject::no_mat );
        glMaterialfv( GL_FRONT, GL_SHININESS, PlanetObject::no_shininess );
        glMaterialfv( GL_FRONT, GL_EMISSION, PlanetObject::mat_emission );
    }
    
    mTexDiffuse.bind( 0 );
	mTexNormal.bind( 1 );
	mTexMask.bind( 2 );
    
    mShader.bind();
    mShader.uniform( "texDiffuse", 0 );
    mShader.uniform( "texNormal", 1 );
    mShader.uniform( "texMask", 2 );
    mShader.uniform( "lightDir", Vec3f(0.0f,0.0f,0.0f) );
    
    //gl::draw( mMesh );
    gl::drawSphere( mPosition, mRadius, 64 );
    
    mShader.unbind();
    mTexNormal.unbind();
    mTexMask.unbind();
    mTexDiffuse.unbind();
    
    glPopAttrib();
}

