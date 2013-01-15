//
// Polyhedron.cpp
// Oculon
//
// Created by Ehsan on 12-01-05.
// Copyright 2012 ewerx. All rights reserved.
//



#include "Constants.h"
#include "Interface.h"
#include "MeshHelper.h"
#include "OculonApp.h"
#include "Polyhedron.h"
#include "Resources.h"
#include "Utils.h"
#include "cinder/CinderMath.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace std;


// ----------------------------------------------------------------
//
Polyhedron::Polyhedron()
: Scene("polyhedron")
{
}

// ----------------------------------------------------------------
//
Polyhedron::~Polyhedron()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::setup()
{
    Scene::setup();
    
    // Params
    mLightEnabled       = true;
    mTextureEnabled     = true;
    mWireframe          = true;
    mDrawInstances      = true;
    mObjectScale        = 10.0f;
    mDivision           = 1;
    mResolution         = 12;
    
    // Set up the instancing grid
	mGridSize		= Vec3i( 16, 16, 16 );
	mGridSpacing	= Vec3f( 2.5f, 2.5f, 2.5f );
    
    // Load shader
	try {
		mShader = gl::GlslProg( loadResource( RES_POLYHEDRON_SHADER_VERT ), loadResource( RES_POLYHEDRON_SHADER_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << ex.what() << endl;
	}
    
	// Load the texture map
	mTexture = gl::Texture( loadImage( loadResource( RES_POLYHEDRON_TEX1 ) ) );
    
    // Set up the light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setAmbient( ColorAf::black() );
	mLight->setDiffuse( ColorAf::gray( 0.6f ) );
	mLight->setDirection( Vec3f::one() );
	mLight->setPosition( Vec3f::one() * -1.0f );
	mLight->setSpecular( ColorAf::white() );
	mLight->enable();
    
    //loadMesh();
    
    createMeshes();
    
    reset();
}

void Polyhedron::loadMesh()
{
    //  ObjLoader loader( (DataSourceRef)loadResource( RES_CUBE_OBJ ) );
    //	loader.load( &mMesh );
    //	mVboMesh = gl::VboMesh( mMesh );
}

void Polyhedron::createMeshes()
{
    mVboMesh = gl::VboMesh( MeshHelper::createIcosahedron( mDivision ) );
}

// ----------------------------------------------------------------
//
void Polyhedron::setupInterface()
{
//    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
//                        .maxValue(CAM_COUNT)
//                        .oscReceiver(getName(), "camera")
//                        .isVertical());
    
    mInterface->addParam(CreateBoolParam( "wireframe", &mWireframe )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "lighting", &mLightEnabled )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "texture", &mTextureEnabled )
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateFloatParam("scale", &mObjectScale)
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName)
                         .sendFeedback());
    
    mInterface->addParam(CreateIntParam("count x", &mGridSize.x)
                         .minValue(0)
                         .maxValue(100)
                         .oscReceiver(mName, "countx")
                         .sendFeedback());
    mInterface->addParam(CreateIntParam("count y", &mGridSize.y)
                         .minValue(0)
                         .maxValue(100)
                         .oscReceiver(mName, "county")
                         .sendFeedback());
    mInterface->addParam(CreateIntParam("count z", &mGridSize.z)
                         .minValue(0)
                         .maxValue(100)
                         .oscReceiver(mName, "countz")
                         .sendFeedback());
    
    mInterface->addParam(CreateFloatParam("spacing x", &mGridSpacing.x)
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName, "spacingx")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("spacing y", &mGridSpacing.y)
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName, "spacingy")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("spacing z", &mGridSpacing.z)
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName, "spacingz")
                         .sendFeedback());
    
//    mInterface->addParam(CreateIntParam("division", &mDivision)
//                         .minValue(1)
//                         .maxValue(100)
//                         .oscReceiver(mName, "division")
//                         .sendFeedback());
//
//    mInterface->addParam(CreateIntParam("resolution", &mResolution)
//                         .minValue(1)
//                         .maxValue(100)
//                         .oscReceiver(mName, "resolution")
//                         .sendFeedback());
}

// ----------------------------------------------------------------
//
void Polyhedron::setupDebugInterface()
{
    Scene::setupDebugInterface();
}

// ----------------------------------------------------------------
//
void Polyhedron::reset()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::resize()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::update(double dt)
{
    // Update light on every frame
	mLight->update( getCamera() );
}

// ----------------------------------------------------------------
//
void Polyhedron::draw()
{
    // Set up OpenGL to work with default lighting
	glShadeModel( GL_SMOOTH );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
    
    // setup
    if ( mLightEnabled ) {
		gl::enable( GL_LIGHTING );
	}
	if ( mTextureEnabled && mTexture ) {
		gl::enable( GL_TEXTURE_2D );
		mTexture.bind();
	}
	if ( mWireframe ) {
		gl::enableWireframe();
	}
    
    // draw
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    gl::scale( Vec3f::one() * mObjectScale );
    
    gl::color( Color::white() );
    
    if (mDrawInstances)
    {
        drawInstances();
    }
    else
    {
        gl::draw( mVboMesh ); // draw one
    }
    
    gl::popMatrices();
    
    // restore
    if ( mWireframe ) {
		gl::disableWireframe();
	}
	if ( mTextureEnabled && mTexture ) {
		mTexture.unbind();
		gl::disable( GL_TEXTURE_2D );
	}
	if ( mLightEnabled ) {
		gl::disable( GL_LIGHTING );
	}
}

void Polyhedron::drawInstances()
{
    // Bind and configure shader
	if ( mShader ) {
		mShader.bind();
		mShader.uniform( "eyePoint",		getCamera().getEyePoint() );
		mShader.uniform( "lightingEnabled",	mLightEnabled );
		mShader.uniform( "size",			Vec3f( mGridSize ) );
		mShader.uniform( "spacing",			mGridSpacing );
		mShader.uniform( "tex",				0 );
		mShader.uniform( "textureEnabled",	mTextureEnabled );
        
        //TODO: texture method
        //TODO: then with dynamic texture
        //TODO: ARB instanced array method -- if textured works don't bother
        //mShader.getAttribLocation("");
	}
    
    // draw instanced
    size_t instanceCount = (size_t)( mGridSize.x * mGridSize.y * mGridSize.z );
    drawInstanced( mVboMesh, instanceCount );

    // Unbind shader
	if ( mShader ) {
		mShader.unbind();
	}
}

// Draw VBO instanced
void Polyhedron::drawInstanced( const gl::VboMesh &vbo, size_t count )
{
	vbo.enableClientStates();
	vbo.bindAllData();
	glDrawElementsInstancedARB( vbo.getPrimitiveType(), vbo.getNumIndices(), GL_UNSIGNED_INT, (GLvoid*)( sizeof(uint32_t) * 0 ), count );
	//glDrawElementsInstancedEXT( vbo.getPrimitiveType(), vbo.getNumIndices(), GL_UNSIGNED_INT, (GLvoid*)( sizeof(uint32_t) * 0 ), count ); // Try this if ARB doesn't work
	gl::VboMesh::unbindBuffers();
	vbo.disableClientStates();
}
