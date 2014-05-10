//
//  Deformer.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 11/10/2013.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "Deformer.h"
#include "MeshHelper.h"
#include "Resources.h"
#include "OculonApp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// ----------------------------------------------------------------
//
Deformer::Deformer()
: Scene("deformer")
{
}

// ----------------------------------------------------------------
//
Deformer::~Deformer()
{
}

#pragma mark - Setup

// ----------------------------------------------------------------
//
void Deformer::setup()
{
    Scene::setup();
    
    // params
	mMeshResolution		= Vec2i( 64, 64 );
	mMeshScale			= Vec3f(100.0f,100.0f,100.0f);
//	mDisplacementSpeed	= 1.0f;
	mDisplacementHeight	= 8.0f;
	mMeshTextureEnabled	= true;
	mWireframe			= true;
    mLightEnabled       = true;
//    mDeformTheta        = 0.0f;
    
//    mShaderTex = loadVertAndFragShaders( "terrain_tex_vert.glsl", "terrain_tex_frag.glsl" );
    mShaderVtf = loadVertAndFragShaders( "terrain_vtf_vert.glsl", "terrain_vtf_frag.glsl" );
    
    //TODO: use geometry shader for single-pass wireframe
    // http://strattonbrazil.blogspot.ca/2011/09/single-pass-wireframe-rendering_10.html
    //mShaderVtf = gl::GlslProg( loadResource( "terrain_vtf_vert.glsl" ), loadResource( "terrain_vtf_frag.glsl" ), loadResource( "wireframe_geom.glsl" ), GL_TRIANGLES, GL_TRIANGLES, 256 );
    
    // Load the texture map
	mMeshTexture = gl::Texture( loadImage( loadResource( "redgreenyellow.png" ) ) );
    mMeshTexture.setWrap( GL_REPEAT, GL_REPEAT );
	
	// Initialize FBO and VBO
	createMesh();
    
    mDynamicTexture.setup(mMeshResolution.x, mMeshResolution.y);
    
    mCameraController.setup(mApp, CameraController::CAM_MANUAL|CameraController::CAM_SPLINE, CameraController::CAM_MANUAL);
}

void Deformer::createMesh()
{
//    // Initialize FBO
//	gl::Fbo::Format format;
//	format.setColorInternalFormat( GL_RGB32F_ARB );
//	mVtfFbo = gl::Fbo( mMeshResolution.x, mMeshResolution.y, format );
//    
//	// Initialize FBO texture
//	mVtfFbo.bindFramebuffer();
//	gl::setViewport( mVtfFbo.getBounds() );
//	gl::clear();
//	mVtfFbo.unbindFramebuffer();
//	mVtfFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
    
	// Generate sphere
	mMesh = gl::VboMesh( MeshHelper::createSphere( mMeshResolution ) );
}

// ----------------------------------------------------------------
//
void Deformer::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "wireframe", &mWireframe )
                        .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam( "texture", &mMeshTextureEnabled )
                        .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam( "lighting", &mLightEnabled )
                         .oscReceiver(mName));
    
    mDynamicTexture.setupInterface(mInterface, mName);
    
    mCameraController.setupInterface(mInterface, mName);
}

// ----------------------------------------------------------------
//
void Deformer::reset()
{
}

#pragma mark - Update

// ----------------------------------------------------------------
//
void Deformer::update(double dt)
{
    Scene::update(dt);
    
//    mDeformTheta += dt;
//    generateDeformationTexture();
    
    mCameraController.update(dt);
    
    mDynamicTexture.update(dt);
}

#pragma mark - Draw

const Camera& Deformer::getCamera()
{
    return mCameraController.getCamera();
}

void Deformer::draw()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT );
    
    glShadeModel( GL_SMOOTH );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::enable( GL_NORMALIZE );
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
    
    glDepthFunc( GL_LESS );
	glEnable( GL_DEPTH_TEST );
    
    // seems to be opposite what it should be... but...
    // GL_FRONT for being inside object
    // GL_BACK for being outside
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
    
    gl::pushMatrices();
    // Set up window
	gl::setViewport( mApp->getViewportBounds() );
	gl::setMatrices( getCamera() );
	//gl::clear( ColorAf::gray( 0.6f ) );
    
	// Use arcball to rotate model view
//	glMultMatrixf( mArcball.getQuat() );
    
	// Enabled lighting, texture mapping, wireframe
	gl::enable( GL_TEXTURE_2D );
//	if ( mLightEnabled ) {
//		gl::enable( GL_LIGHTING );
//	}
	
	if ( mWireframe )
    {
		gl::enableWireframe();
	}
    
	// Bind textures
	mDynamicTexture.bindTexture(0);
	mMeshTexture.bind( 1 );
    
	// Bind and configure displacement shader
	mShaderVtf.bind();
	mShaderVtf.uniform( "displacement", 0 );
	mShaderVtf.uniform( "eyePoint", getCamera().getEyePoint() );
	mShaderVtf.uniform( "height", mDisplacementHeight );
	mShaderVtf.uniform( "lightingEnabled", mLightEnabled );
	mShaderVtf.uniform( "scale", mMeshScale );
	mShaderVtf.uniform( "tex", 1 );
	mShaderVtf.uniform( "textureEnabled", mMeshTextureEnabled );
    
	// Draw sphere
	gl::draw( mMesh );
	
	// Unbind everything
	mShaderVtf.unbind();
	mMeshTexture.unbind();
	mDynamicTexture.unbindTexture();
	
	// Disable wireframe, texture mapping, lighting
	if ( mWireframe ) {
		gl::disableWireframe();
	}
	if ( mMeshTextureEnabled && mMeshTexture ) {
		mMeshTexture.unbind();
	}
	if ( mLightEnabled ) {
		gl::disable( GL_LIGHTING );
	}
	gl::disable( GL_TEXTURE_2D );
    
    gl::popMatrices();
    glPopAttrib();
}

void Deformer::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw( mDynamicTexture.getTexture(), preview );
    
    glPopAttrib();
    gl::popMatrices();
}

#pragma mark Deformation Texture

//void Deformer::generateDeformationTexture()
//{
//    gl::pushMatrices();
//    // Bind FBO and set up window
//	mVtfFbo.bindFramebuffer();
//	gl::setViewport( mVtfFbo.getBounds() );
//	gl::setMatricesWindow( mVtfFbo.getSize() );
//	gl::clear();
//    
//	// Bind and configure dynamic texture shader
//	mShaderTex.bind();
//	mShaderTex.uniform( "theta", mDeformTheta );
//    
//	// Draw shader output
//	gl::enable( GL_TEXTURE_2D );
//	gl::color( Colorf::white() );
//	gl::begin( GL_TRIANGLES );
//    
//	// Define quad vertices
//	Vec2f vert0( (float)mVtfFbo.getBounds().x1, (float)mVtfFbo.getBounds().y1 );
//	Vec2f vert1( (float)mVtfFbo.getBounds().x2, (float)mVtfFbo.getBounds().y1 );
//	Vec2f vert2( (float)mVtfFbo.getBounds().x1, (float)mVtfFbo.getBounds().y2 );
//	Vec2f vert3( (float)mVtfFbo.getBounds().x2, (float)mVtfFbo.getBounds().y2 );
//    
//	// Define quad texture coordinates
//	Vec2f uv0( 0.0f, 0.0f );
//	Vec2f uv1( 1.0f, 0.0f );
//	Vec2f uv2( 0.0f, 1.0f );
//	Vec2f uv3( 1.0f, 1.0f );
//    
//	// Draw quad (two triangles)
//	gl::texCoord( uv0 );
//	gl::vertex( vert0 );
//	gl::texCoord( uv2 );
//	gl::vertex( vert2 );
//	gl::texCoord( uv1 );
//	gl::vertex( vert1 );
//    
//	gl::texCoord( uv1 );
//	gl::vertex( vert1 );
//	gl::texCoord( uv2 );
//	gl::vertex( vert2 );
//	gl::texCoord( uv3 );
//	gl::vertex( vert3 );
//    
//	gl::end();
//	gl::disable( GL_TEXTURE_2D );
//    
//	// Unbind everything
//	mShaderTex.unbind();
//	mVtfFbo.unbindFramebuffer();
//    gl::popMatrices();
//}
