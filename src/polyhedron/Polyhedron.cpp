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
    mCamType            = CAM_SPLINE;
    mLineWidth          = 2.0f;
    mColor              = ColorAf::white();
    
    // Set up the instancing grid
	mGridSize           = Vec3i( 16, 16, 16 );
	mGridSpacing        = Vec3f( 2.5f, 2.5f, 2.5f );
    
    mMeshType           = MESH_TYPE_ICOSAHEDRON;
    
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
    
    // camera
    mSplineCam.setup(5000.0f);
    
    //loadMesh();
    createMeshes();
    
    // Audio
    mFalloff = 0.32f;
    mFalloffMode = FALLOFF_OUTBOUNCE;
    mAudioRowShift = 0;
    mAudioRowShiftTime = 0.0f;
    mAudioRowShiftDelay = 0.25f;
    mDisplacementScale = 1.0f;
    
    // AUDIO
    mAudioFboDim    = 16; // 256 bands
    mAudioFboSize   = Vec2f( mAudioFboDim, mAudioFboDim );
    mAudioFboBounds = Area( 0, 0, mAudioFboDim, mAudioFboDim );
    gl::Fbo::Format audioFboFormat;
	audioFboFormat.setColorInternalFormat( GL_RGB32F_ARB );
    mAudioFbo       = gl::Fbo( mAudioFboDim, mAudioFboDim, audioFboFormat );
    
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
    //mCircle			= gl::VboMesh( MeshHelper::createCircle( Vec2i( mResolution, mResolution ) ) );
	//mCone			= gl::VboMesh( MeshHelper::createCylinder( Vec2i( mResolution, mResolution ), 0.0f, 1.0f, false, true ) );
	mCube			= gl::VboMesh( MeshHelper::createCube( Vec3i( 4, 4, 4 ) ) );
	//mCylinder		= gl::VboMesh( MeshHelper::createCylinder( Vec2i( mResolution, mResolution ) ) );
	mIcosahedron	= gl::VboMesh( MeshHelper::createIcosahedron( mDivision ) );
	//mRing			= gl::VboMesh( MeshHelper::createRing( Vec2i( mResolution, mResolution ) ) );
	mSphere			= gl::VboMesh( MeshHelper::createSphere( Vec2i( 8, 8 ) ) );
	//mSquare			= gl::VboMesh( MeshHelper::createSquare( Vec2i( mResolution, mResolution ) ) );
	//mTorus			= gl::VboMesh( MeshHelper::createTorus( Vec2i( mResolution, mResolution ) ) );
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
    
    vector<string> meshTypeNames;
#define POLYHEDRON_MESHTYPE_ENTRY( nam, enm ) \
    meshTypeNames.push_back(nam);
    POLYHEDRON_MESHTYPE_TUPLE
#undef  POLYHEDRON_MESHTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Mesh", (int*)(&mMeshType) )
                        .maxValue(MESH_COUNT)
                        .oscReceiver(getName(), "mesh")
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
    
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define POLYHEDRON_CAMTYPE_ENTRY( nam, enm ) \
    camTypeNames.push_back(nam);
    POLYHEDRON_CAMTYPE_TUPLE
#undef  POLYHEDRON_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical(), camTypeNames);
    mSplineCam.setupInterface(mInterface, mName);
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
    Scene::update(dt);
    
    if( mCamType == CAM_SPLINE )
        mSplineCam.update(dt);
    
    mAudioRowShiftTime += dt;
    
    if (mDynamicLight)
    {
        mLight->setPosition( getCamera().getEyePoint() * 1.1f );
        mLight->setDirection( getCamera().getCenterOfInterestPoint() );
    }
    
    // Update light on every frame
	mLight->update( getCamera() );
}

#pragma mark Audio

void Polyhedron::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
	
    // Get data
    //float * freqData = audioInput.getFft()->getAmplitude();
    //float * timeData = audioInput.getFft()->getData();
    int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    if( mFftFalloff.size() == 0 )
    {
        for( int i=0; i< dataSize; ++i )
        {
            mFftFalloff.push_back( fftLogData[i].y );
        }
    }
    
    int32_t row = mAudioRowShift;
    
    Rand randIndex(0);
	Surface32f fftSurface( mAudioFbo.getTexture() );
	Surface32f::Iter it = fftSurface.getIter();
    int32_t index = 0;
	while( it.line() )
    {
        //int32_t index = row * mAudioFboDim;
		while( it.pixel() && index < dataSize )
        {
            int32_t bandIndex = Rand::randInt(dataSize);//randIndex.nextInt(dataSize);
            if (fftLogData[bandIndex].y > mFftFalloff[index])
            {
                //mFftFalloff[index] = fftLogData[bandIndex].y;
                timeline().apply( &mFftFalloff[index], fftLogData[bandIndex].y, mFalloff/2.0f, getReverseFalloffFunction() );
                timeline().appendTo(&mFftFalloff[index], 0.0f, mFalloff, getReverseFalloffFunction() );
                //timeline().apply( &mFftFalloff[index], 0.0f, mFalloff, getFalloffFunction() );
            } else if (fftLogData[bandIndex].y < mFftFalloff[index]) {
                timeline().apply( &mFftFalloff[index], 0.0f, mFalloff, getFalloffFunction() );
            }
            
			it.r() = mFftFalloff[index]();
            it.g() = 0.0f; // UNUSED
			it.b() = 0.0f; // UNUSED
			it.a() = 1.0f; // UNUSED
            
            ++index;
		}
        
        ++row;
        if (row >= mAudioFboDim)
        {
            row = 0;
        }
	}
    
    if (mAudioRowShiftTime >= mAudioRowShiftDelay)
    {
        mAudioRowShiftTime = 0.0f;
        ++mAudioRowShift;
        if (mAudioRowShift >= mAudioFboDim) {
            mAudioRowShift = 0;
        }
    }
	
	gl::Texture fftTexture( fftSurface );
	mAudioFbo.bindFramebuffer();
	gl::setMatricesWindow( mAudioFboSize, false );
	gl::setViewport( mAudioFboBounds );
	gl::draw( fftTexture );
	mAudioFbo.unbindFramebuffer();
}

Polyhedron::tEaseFn Polyhedron::getFalloffFunction()
{
    switch( mFalloffMode )
    {
        case FALLOFF_LINEAR: return EaseNone();
        case FALLOFF_OUTQUAD: return EaseOutQuad();
        case FALLOFF_OUTEXPO: return EaseOutExpo();
        case FALLOFF_OUTBACK: return EaseOutBack();
        case FALLOFF_OUTBOUNCE: return EaseOutBounce();
        case FALLOFF_OUTINEXPO: return EaseOutInExpo();
        case FALLOFF_OUTINBACK: return EaseOutInBack();
            
        default: return EaseNone();
    }
}

Polyhedron::tEaseFn Polyhedron::getReverseFalloffFunction()
{
    switch( mFalloffMode )
    {
        case FALLOFF_LINEAR: return EaseNone();
        case FALLOFF_OUTQUAD: return EaseInQuad();
        case FALLOFF_OUTEXPO: return EaseInExpo();
        case FALLOFF_OUTBACK: return EaseInBack();
        case FALLOFF_OUTBOUNCE: return EaseInBounce();
        case FALLOFF_OUTINEXPO: return EaseInOutExpo();
        case FALLOFF_OUTINBACK: return EaseInOutBack();
            
        default: return EaseNone();
    }
}


// ----------------------------------------------------------------
//
void Polyhedron::draw()
{
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
    if ( mLightEnabled ) {
		gl::enable( GL_LIGHTING );
        Color diffuse = mColor * mColor.a;
        mLight->setDiffuse( diffuse );
        mLight->setSpecular( mColor );
	}
	if ( mTextureEnabled && mTexture ) {
		gl::enable( GL_TEXTURE_2D );
		mTexture.bind();
	}
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
    
    // restore
    if ( mWireframe ) {
		gl::disableWireframe();
        glLineWidth( 1.0f );
	}
	if ( mTextureEnabled && mTexture ) {
		mTexture.unbind();
		gl::disable( GL_TEXTURE_2D );
	}
	if ( mLightEnabled ) {
		gl::disable( GL_LIGHTING );
	}
}

void Polyhedron::drawInstances( const ci::gl::VboMesh &mesh )
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
        //mShader.uniform( "displacement",    1 );
        //mShader.uniform( "dispScale",       mDisplacementScale );
        
        //TODO: texture method
        //TODO: then with dynamic texture
        //TODO: ARB instanced array method -- if textured works don't bother
        //mShader.getAttribLocation("");
	}
    
    // draw instanced
    size_t instanceCount = (size_t)( mGridSize.x * mGridSize.y * mGridSize.z );
    drawInstanced( mesh, instanceCount );

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

void Polyhedron::drawDebug()
{
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    mAudioFbo.bindTexture();
    //TODO: make utility func for making rects with origin/size
    gl::drawSolidRect( Rectf( 100.0f, mApp->getWindowHeight() - 120.0f, 180.0f, mApp->getWindowHeight() - 40.0f ) );
    
    gl::disable( GL_TEXTURE_2D );
}

const Camera& Polyhedron::getCamera()
{
    switch( mCamType )
    {
        case CAM_SPLINE:
            return mSplineCam.getCamera();
            
        case CAM_CATALOG:
        {
            Scene* scene = mApp->getScene("catalog");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mSplineCam.getCamera();
            }
        }
            
        case CAM_GRAVITON:
        {
            Scene* scene = mApp->getScene("graviton");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mSplineCam.getCamera();
            }
        }
            
        case CAM_ORBITER:
        {
            Scene* scene = mApp->getScene("orbiter");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mSplineCam.getCamera();
            }
        }
            
        default:
            return mApp->getMayaCam();
    }
}

const ci::gl::VboMesh& Polyhedron::getMesh()
{
    switch( mMeshType )
    {
//        case MESH_TYPE_CIRCLE:
//            return mCircle;
//        case MESH_TYPE_CONE:
//            return mCone;
//        case MESH_TYPE_CYLINDER:
//            return mCylinder;
//        case MESH_TYPE_RING:
//            return mRing;
//        case MESH_TYPE_SQUARE:
//            return mSquare;
//        case MESH_TYPE_TORUS:
//            return mTorus;
        case MESH_TYPE_ICOSAHEDRON:
            return mIcosahedron;
        case MESH_TYPE_SPHERE:
            return mSphere;
        case MESH_TYPE_CUBE:
            return mCube;
//        case MESH_TYPE_CUSTOM:
//            return mCustom;
            
        default:
            console() << "[polyhedron] invalid mesh type" << std::endl;
            return mIcosahedron;
	}
}
