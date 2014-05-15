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
#include <vector>

using namespace ci;
using namespace ci::app;
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
    mTextureEnabled     = false;
    mWireframe          = true;
    mDrawInstances      = true;
    mAdditiveBlending   = true;
    mDynamicLight       = false;
    mObjectScale        = 10.0f;
    mDivision           = 1;
    mResolution         = 12;
    mLineWidth          = 2.0f;
    mColor              = ColorAf::white();
    
    // Set up the instancing grid
	mGridSize           = Vec3i( 4, 4, 4 );
	mGridSpacing        = Vec3f( 2.5f, 2.5f, 2.5f );
    mNumObjects         = mGridSize.x * mGridSize.y * mGridSize.z;
    
    mMeshType           = 0;
    
    // Load shader
	mShader = loadVertAndFragShaders("polyhedron_vert.glsl", "polyhedron_frag.glsl");
    
	// Load the texture map
	mTexture = gl::Texture( loadImage( loadResource( "redgreenyellow.png" ) ) );
    
    // Set up the light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setAmbient( ColorAf::black() );
	mLight->setDiffuse( ColorAf::gray( 0.6f ) );
	mLight->setDirection( Vec3f::one() );
	mLight->setPosition( Vec3f::one() * -1.0f );
	mLight->setSpecular( ColorAf::white() );
	mLight->enable();
    
    // displacement texture
    mDynamicTexture.setup(mGridSize.x, mGridSize.y);
    
    //loadMesh();
    createMeshes();
    
    // camera
    mCameraController.setup(mApp, CameraController::CAM_MANUAL|CameraController::CAM_SPLINE, CameraController::CAM_MANUAL);
    
    // audio
    mAudioInputHandler.setup(true);
    
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
    gl::VboMesh icosahedron	= gl::VboMesh( MeshHelper::createIcosahedron( mDivision ) );
    mMeshes.push_back( make_pair( "icosahedron", icosahedron ) );
    
	gl::VboMesh cube = gl::VboMesh( MeshHelper::createCube( Vec3i( 4, 4, 4 ) ) );
    mMeshes.push_back( make_pair( "cube", cube ) );
    
    gl::VboMesh sphere = gl::VboMesh( MeshHelper::createSphere( Vec2i( 8, 8 ) ) );
    mMeshes.push_back( make_pair( "sphere", sphere ) );
    
    gl::VboMesh torus = gl::VboMesh( MeshHelper::createTorus( Vec2i( mResolution, mResolution ) ) );
    mMeshes.push_back( make_pair( "torus", torus ) );
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
    mInterface->addParam(CreateBoolParam( "additive", &mAdditiveBlending )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "dynamic light", &mDynamicLight )
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
                         .sendFeedback())->registerCallback(this, &Polyhedron::onGridSizeChange);
    mInterface->addParam(CreateIntParam("count y", &mGridSize.y)
                         .minValue(0)
                         .maxValue(100)
                         .oscReceiver(mName, "county")
                         .sendFeedback())->registerCallback(this, &Polyhedron::onGridSizeChange);
    mInterface->addParam(CreateIntParam("count z", &mGridSize.z)
                         .minValue(0)
                         .maxValue(100)
                         .oscReceiver(mName, "countz")
                         .sendFeedback())->registerCallback(this, &Polyhedron::onGridSizeChange);
    
//    mInterface->addParam(CreateIntParam("total objects", &mNumObjects)
//                         .maxValue(100000)
//                         .sendFeedback()); // read-only
    
    mInterface->addParam(CreateFloatParam("spacing x", &mGridSpacing.x)
                         .minValue(0.0f)
                         .maxValue(10.0f)
                         .oscReceiver(mName, "spacingx")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("spacing y", &mGridSpacing.y)
                         .minValue(0.0f)
                         .maxValue(10.0f)
                         .oscReceiver(mName, "spacingy")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("spacing z", &mGridSpacing.z)
                         .minValue(0.0f)
                         .maxValue(10.0f)
                         .oscReceiver(mName, "spacingz")
                         .sendFeedback());
    
    mInterface->addParam(CreateIntParam("line width", &mLineWidth)
                         .minValue(1)
                         .maxValue(8)
                         .oscReceiver(mName, "linewidth")
                         .sendFeedback());
    
    mInterface->gui()->addColumn();
    vector<string> meshTypeNames;
    for( tNamedMesh namedMesh : mMeshes )
    {
        meshTypeNames.push_back(namedMesh.first);
    }
    mInterface->addEnum(CreateEnumParam( "mesh", (int*)(&mMeshType) )
                        .maxValue(mMeshes.size())
                        .oscReceiver(getName())
                        .isVertical(), meshTypeNames);
    
    mInterface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color"));
    
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
    mDynamicTexture.setupInterface(mInterface, mName);
    
    mCameraController.setupInterface(mInterface, mName);
    mAudioInputHandler.setupInterface(mInterface, mName);
}

// ----------------------------------------------------------------
//
//void Polyhedron::setupDebugInterface()
//{
//    Scene::setupDebugInterface();
//}

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

#pragma mark - callbacks

bool Polyhedron::onGridSizeChange()
{
    mNumObjects = mGridSize.x * mGridSize.y * mGridSize.z;
    return false;
}

#pragma mark - update

// ----------------------------------------------------------------
//
void Polyhedron::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    mDynamicTexture.update(dt);
    
    if (mDynamicLight)
    {
        mLight->setPosition( getCamera().getEyePoint() * 1.1f );
        mLight->setDirection( getCamera().getCenterOfInterestPoint() );
    }
    
    // Update light on every frame
	mLight->update( getCamera() );
}

#pragma mark - draw

// ----------------------------------------------------------------
//
void Polyhedron::draw()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Set up OpenGL to work with default lighting
	glShadeModel( GL_SMOOTH );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    
    if (mAdditiveBlending)
    {
        gl::enableAdditiveBlending();
    }
    else
    {
        gl::enableAlphaBlending();
	}
    gl::enableDepthRead();
	gl::enableDepthWrite();
    
    // setup
    if ( mLightEnabled )
    {
		gl::enable( GL_LIGHTING );
        Color diffuse = mColor * mColor.a;
        mLight->setDiffuse( diffuse );
        mLight->setSpecular( mColor );
	}
    
	if ( mTextureEnabled && mTexture )
    {
		gl::enable( GL_TEXTURE_2D );
		mTexture.bind(0);
	}
    
    mDynamicTexture.bindTexture(1);
    
	if ( mWireframe )
    {
        glLineWidth( (GLfloat)mLineWidth );
		gl::enableWireframe();
	}
    
    // draw
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    gl::scale( Vec3f::one() * mObjectScale );
    
    gl::color( mColor );
    
    if (mDrawInstances)
    {
        drawInstances( getMesh() );
    }
    else
    {
        gl::draw( getMesh() ); // draw one
    }
    
    gl::popMatrices();
    
    mDynamicTexture.unbindTexture();
    
    // restore
    if ( mWireframe )
    {
		gl::disableWireframe();
        glLineWidth( 1.0f );
	}
    
	if ( mTextureEnabled && mTexture )
    {
		mTexture.unbind();
		gl::disable( GL_TEXTURE_2D );
	}
    
	if ( mLightEnabled )
    {
		gl::disable( GL_LIGHTING );
	}
    
    glPopAttrib();
}

void Polyhedron::drawInstances( const ci::gl::VboMesh &mesh )
{
    // config shader
    mShader.bind();
    mShader.uniform( "eyePoint",		getCamera().getEyePoint() );
    mShader.uniform( "lightingEnabled",	mLightEnabled );
    mShader.uniform( "size",			Vec3f( mGridSize ) );
    mShader.uniform( "spacing",			mGridSpacing );
    mShader.uniform( "tex",				0 );
    mShader.uniform( "textureEnabled",	mTextureEnabled );
    mShader.uniform( "displacement",    1 );
//    mShader.uniform( "dispScale",       mDisplacementScale );
    
    //TODO: texture method
    //TODO: then with dynamic texture
    //TODO: ARB instanced array method -- if textured works don't bother
    //mShader.getAttribLocation("");
    
    // draw instanced
    size_t instanceCount = (size_t)( mGridSize.x * mGridSize.y * mGridSize.z );
    drawInstanced( mesh, instanceCount );
    
    mShader.unbind();
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

void Polyhedron::drawDebug()
{
    //Scene::drawDebug();
    
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    gl::enable( GL_TEXTURE_2D );
    const Vec2f& windowSize( getWindowSize() );
    gl::setMatricesWindow( windowSize );
    
    const float size = 80.0f;
    const float paddingX = 20.0f;
    const float paddingY = 240.0f;
    
    Rectf preview( windowSize.x - (size+paddingX), windowSize.y - (size+paddingY), windowSize.x-paddingX, windowSize.y - paddingY );
    gl::draw(mDynamicTexture.getTexture(), preview);
    
    glPopAttrib();
    gl::popMatrices();
}

const Camera& Polyhedron::getCamera()
{
    return mCameraController.getCamera();
}

const ci::gl::VboMesh& Polyhedron::getMesh()
{
    assert(mMeshType < mMeshes.size());
    
    return mMeshes[mMeshType].second;
}
