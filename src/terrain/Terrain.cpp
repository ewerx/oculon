//
// Terrain.cpp
// Oculon
//
// Created by Ehsan on 13-Mar-25.
// Copyright 2013 ewerx. All rights reserved.
//

#include "Constants.h"
#include "Interface.h"
#include "MeshHelper.h"
#include "OculonApp.h"
#include "Terrain.h"
#include "Resources.h"
#include "Utils.h"
#include "RDiffusion.h"
#include "cinder/CinderMath.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"
#include <vector>

using namespace ci;
using namespace std;

#define VTF_FBO_SIZE 64


// ----------------------------------------------------------------
//
Terrain::Terrain()
: Scene("terrain")
{
}

// ----------------------------------------------------------------
//
Terrain::~Terrain()
{
}

// ----------------------------------------------------------------
//
void Terrain::setup()
{
    
    // Params
    mCamType = CAM_STATIC;
    mDrawWireframe = true;
	mDrawFlatShaded = true;
    mMeshFaceAlpha = 1.0f;
    mMeshLineAlpha = 1.0f;
	mDrawShadowMap = false;
    mMeshType = MESHTYPE_FLAT;
    mEnableLight = false;
    mEnableShadow = false;
    mEnableTexture = false;
    mDrawMirror = false;
    mNoiseScale = Vec3f(1.0f,1.0f,0.25f);
    mDisplacementMode = DISPLACE_NOISE;
    
    // Audio
    mFalloff = 0.32f;
    mFalloffMode = FALLOFF_OUTBOUNCE;
    mAudioRowShift = 0;
    mAudioRowShiftTime = 0.0f;
    mAudioRowShiftDelay = 0.25f;
    mAudioEffectNoise = false;
    
    // VTF
    // Initialize FBO
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
	mVtfFbo = gl::Fbo( VTF_FBO_SIZE, VTF_FBO_SIZE, format );
    
	// Initialize FBO texture
	mVtfFbo.bindFramebuffer();
	gl::setViewport( mVtfFbo.getBounds() );
	gl::clear();
	mVtfFbo.unbindFramebuffer();
	mVtfFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
    
    // Tube BSpline
	mBasePoints.push_back( Vec3f( -3,  4, 0 ) );
	mBasePoints.push_back( Vec3f(  5,  1, 0 ) );
	mBasePoints.push_back( Vec3f( -5, -1, 0 ) );
	mBasePoints.push_back( Vec3f(  3, -4, 0 ) );
	mCurPoints = mBasePoints;
    
    // create the mesh
	setupMesh();
    
	// initialize shadow map
	setupShadowMap();
    
	// load the texture from the "assets" folder (easier than using resources)
	try
    {
        mTexture = gl::Texture( loadImage( loadResource(RES_TERRAIN_TEXTURE) ) );
    }
	catch( const std::exception &e )
    {
        console() << "Could not load texture:" << e.what() << std::endl;
    }
    
	// load and compile the shader
	try
    {
        mShader = gl::GlslProg( loadResource(RES_TERRAIN_VERT), loadResource(RES_TERRAIN_FRAG) );
    }
	catch( const std::exception &e )
    {
        console() << "Could not load&compile shader:" << e.what() << std::endl;
    }
//#endif
    
    setupDynamicTexture();
    
    // CAMERA
    mSplineCam.setup(1000.0f, 300.0f);
    mScale = 620.0f;
    mTunnelDistance = -300.0f;
    mTunnelCam.setup( Vec3f( 0.0f, mTunnelDistance, 0.0f ), true );
    mTunnelCam.mRadius = 200.0f;
    mStaticCamPos = Vec3f( 220.0f, 1.0f, 255.0f );
	mStaticCam.lookAt( mStaticCamPos, Vec3f( 0.0f, 0.0f, 0.0f ) );

    //mApp->setCamera(Vec3f( 0.0f, 40.0f, 0.0f ), Vec3f( 0.0f, 0.0f, 0.0f ), Vec3f(1.0f,1.0f,0.0f));
    
    // LIGHT
    gl::enable( GL_LIGHTING );
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setAmbient( ColorAf::black() );
	mLight->setDiffuse( ColorAf::black() );
	mLight->setDirection( Vec3f::one() );
	mLight->setPosition( Vec3f::one() * -1.0f );
	mLight->setSpecular( ColorAf::white() );
	mLight->enable();
    
    // AUDIO
    mAudioFboDim    = 16; // 256 bands
    mAudioFboSize   = Vec2f( mAudioFboDim, mAudioFboDim );
    mAudioFboBounds = Area( 0, 0, mAudioFboDim, mAudioFboDim );
    gl::Fbo::Format audioFboFormat;
	audioFboFormat.setColorInternalFormat( GL_RGB32F_ARB );
    mAudioFbo       = gl::Fbo( mAudioFboDim, mAudioFboDim, audioFboFormat );
    
    // Reaction Diffusion
    mRDiffusion = RDiffusion( VTF_FBO_SIZE, VTF_FBO_SIZE );
    
    Scene::setup();
    reset();
}

// ----------------------------------------------------------------
//
void Terrain::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "wireframe", &mDrawWireframe )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "flat_shaded", &mDrawFlatShaded )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "face_alpha", &mMeshFaceAlpha )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "line_alpha", &mMeshLineAlpha )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "mirror", &mDrawMirror )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "lighting", &mEnableLight )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "shadow", &mEnableShadow )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "texture", &mEnableTexture )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "disp_speed", &mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "disp_height", &mDisplacementHeight() )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,50.0f))
                         .oscReceiver(mName));
//    mInterface->addParam(CreateBoolParam( "shadow map", &mDrawShadowMap )
//                         .oscReceiver(getName()));

    vector<string> dispTypeNames;
#define TERRAIN_DISPLACEMODE_ENTRY( nam, enm ) \
    dispTypeNames.push_back(nam);
    TERRAIN_DISPLACEMODE_TUPLE
#undef  TERRAIN_DISPLACEMODE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Displacement", (int*)(&mDisplacementMode) )
                        .maxValue(DISPLACE_COUNT)
                        .oscReceiver(getName(), "displacement")
                        .isVertical(), dispTypeNames);

    vector<string> meshTypeNames;
#define TERRAIN_MESHTYPE_ENTRY( nam, enm ) \
    meshTypeNames.push_back(nam);
    TERRAIN_MESHTYPE_TUPLE
#undef  TERRAIN_MESHTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Mesh", (int*)(&mMeshType) )
                        .maxValue(MESHTYPE_COUNT)
                        .oscReceiver(getName(), "meshtype")
                        .isVertical(), meshTypeNames)->registerCallback( this, &Terrain::setupMesh );
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateBoolParam( "Audio Noise", &mAudioEffectNoise )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("Falloff", &mFalloff)
                        .minValue(0.0f)
                        .maxValue(20.0f));
    mInterface->addEnum(CreateEnumParam("Falloff Mode", (int*)&mFalloffMode)
                       .maxValue(FALLOFF_COUNT)
                       .oscReceiver(mName,"falloffmode")
                       .isVertical());
    mInterface->addParam(CreateFloatParam("Shift Delay", &mAudioRowShiftDelay)
                         .minValue(0.0f)
                         .maxValue(1.0f));
    
    mInterface->addParam(CreateFloatParam("Scale", &mScale)
                         .minValue(400.0f)
                         .maxValue(1000.0f));
    mInterface->addParam(CreateFloatParam("Cam Radius", &mTunnelCam.mRadius)
                         .minValue(100.0f)
                         .maxValue(1000.0f));

    
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define TERRAIN_CAMTYPE_ENTRY( nam, enm ) \
    camTypeNames.push_back(nam);
    TERRAIN_CAMTYPE_TUPLE
#undef  TERRAIN_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical(), camTypeNames);
    mInterface->addParam(CreateVec3fParam("static_cam_pos", &mStaticCamPos, Vec3f(200.0f,-50.0f,200.0f), Vec3f(260.0f, 50.0f, 260.0f))
                         .oscReceiver(getName()))->registerCallback( this, &Terrain::updateStaticCamPos );
    mSplineCam.setupInterface(mInterface, mName);
    mTunnelCam.setupInterface(mInterface, mName);
}

// ----------------------------------------------------------------
//
void Terrain::setupDebugInterface()
{
    Scene::setupDebugInterface();
}

// ----------------------------------------------------------------
//
void Terrain::reset()
{
}

// ----------------------------------------------------------------
//
void Terrain::resize()
{
}

#pragma mark - Update

// ----------------------------------------------------------------
//
void Terrain::update(double dt)
{
    Scene::update(dt);
    
    gl::disableAlphaBlending();
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::color( Color( 1, 1, 1 ) );
    
    mAudioRowShiftTime += dt;
    updateAudioResponse();
    
    // update noise
	float time = (float)getElapsedSeconds() * mDisplacementSpeed;
	mTheta = time;//math<float>::sin( time );
    drawDynamicTexture();
    
    if( mCamType == CAM_SPLINE )
    {
        mSplineCam.update(dt);
    }
    
    //if( mCamType == CAM_TUNNEL )
    {
        mTunnelCam.update(dt);
        
//        const Vec3f& tunnelPos = mTunnelCam.getPosition();
//        console() << "tunnel cam y = " << tunnelPos.y << std::endl;
//        
//        if( tunnelPos.y >= 100.0f )
//        {
//            mCurMesh = ( mCurMesh + 1 ) % 2;
//            mNextMesh = ( mCurMesh + 1 ) % 2;
//            
//            //mTunnelCam.reset();
//            console() << "tunnel reset!" << std::endl;
//        }
    }
    
    if (mEnableShadow) {
        // animate light
        float x = 50.0f + 150.0f * (float) sin( 0.20 * getElapsedSeconds() );
        float y = 50.0f +  45.0f * (float) cos( 0.13 * getElapsedSeconds() );
        float z = 50.0f + 150.0f * (float) cos( 0.20 * getElapsedSeconds() );
        
        mLightPosition = Vec3f(x, y, z);
    } else {
        mLight->update( getCamera() );
    }
}


void Terrain::drawDynamicTexture()
{
    if (mAudioEffectNoise)
    {
        
    }
    gl::pushMatrices();
    
    // Bind FBO and set up window
	mVtfFbo.bindFramebuffer();
	gl::setViewport( mVtfFbo.getBounds() );
	gl::setMatricesWindow( mVtfFbo.getSize() );
	gl::clear();
    
	// Bind and configure dynamic texture shader
	mShaderTex.bind();
	mShaderTex.uniform( "theta", mTheta );
    mShaderTex.uniform( "scale", mNoiseScale );
    //    mShaderTex.uniform( "u_time", mTheta );
    //    mShaderTex.uniform( "u_scale", 1.0f );
    //    mShaderTex.uniform( "u_RenderSize", mVtfFbo.getSize() );
    
	// Draw shader output
	gl::enable( GL_TEXTURE_2D );
	gl::color( Colorf::white() );
	gl::begin( GL_TRIANGLES );
    
	// Define quad vertices
	Vec2f vert0( (float)mVtfFbo.getBounds().x1, (float)mVtfFbo.getBounds().y1 );
	Vec2f vert1( (float)mVtfFbo.getBounds().x2, (float)mVtfFbo.getBounds().y1 );
	Vec2f vert2( (float)mVtfFbo.getBounds().x1, (float)mVtfFbo.getBounds().y2 );
	Vec2f vert3( (float)mVtfFbo.getBounds().x2, (float)mVtfFbo.getBounds().y2 );
    
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
    gl::disable( GL_TEXTURE_2D );
    
	// Unbind everything
	mShaderTex.unbind();
	mVtfFbo.unbindFramebuffer();
    
    gl::popMatrices();
    
	///////////////////////////////////////////////////////////////
}

bool Terrain::updateStaticCamPos()
{
    mStaticCam.setEyePoint( mStaticCamPos );
    return false;
}

#pragma mark Audio

void Terrain::updateAudioResponse()
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
	
    gl::pushMatrices();
	gl::Texture fftTexture( fftSurface );
	mAudioFbo.bindFramebuffer();
	gl::setMatricesWindow( mAudioFboSize, false );
	gl::setViewport( mAudioFboBounds );
	gl::draw( fftTexture );
	mAudioFbo.unbindFramebuffer();
    gl::popMatrices();
    
    if (mAudioEffectNoise)
    {
        float newHeight = 20.0f * audioInput.getAverageVolumeByFrequencyRange();
        if (newHeight > mDisplacementHeight) {
            timeline().apply( &mDisplacementHeight, newHeight, mFalloff/2.0f, getReverseFalloffFunction() );
            timeline().appendTo(&mDisplacementHeight, 0.0f, mFalloff, getFalloffFunction() );
        }
    }
}

Terrain::tEaseFn Terrain::getFalloffFunction()
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

Terrain::tEaseFn Terrain::getReverseFalloffFunction()
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

#pragma mark - Render 

// ----------------------------------------------------------------
//
void Terrain::draw()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    gl::pushMatrices();
	gl::setMatrices( getCamera() );
    
    // VTF
    // Set up OpenGL to work with default lighting
    if (!mDrawFlatShaded) {
        glShadeModel( GL_SMOOTH );
    } else {
        glShadeModel( GL_FLAT );
    }
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::enable( GL_NORMALIZE );
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
    
    drawMesh();
    
	// no need to call 'disableLights()' or 'gl::disableDepthRead/Write()' here,
	// glPopAttrib() will do it for us
    
	gl::popMatrices();
    // restore OpenGL state
	glPopAttrib();
}

void Terrain::drawMesh()
{
	// Set up window
	gl::setViewport( mApp->getViewportBounds() );
	gl::setMatrices( getCamera() );
    
    // DEBUG DRAW
//    if (mCamType != CAM_TUNNEL) {
//        gl::color(1.0f,0.0f,0.0f,1.0f);
//        gl::drawSphere(mTunnelCam.getCamera().getEyePoint(), 10.0f);
//        gl::drawLine(mTunnelCam.getCamera().getEyePoint(), mTunnelCam.getCamera().getCenterOfInterestPoint());
//    }
    
    // draw light position
	if(mDrawShadowMap) {
		gl::color( Color(1.0f, 1.0f, 0.0f) );
		gl::drawFrustum( mShadowCamera );
	}
    
//    gl::color(1.0f,1.0f,0.0f,1.0f);
//    gl::drawSphere(Vec3f(0.f,-100.f,0.f), 10.0f);
//    gl::color(0.0f,1.0f,1.0f,1.0f);
//    gl::drawSphere(Vec3f(0.f,100.f,0.f), 10.0f);
    
    // THE REAL DEAL
	gl::enable( GL_TEXTURE_2D );
    
    if(mEnableLight) {
        enableLights();
    }
    else {
        disableLights();
    }
    
	// Bind textures
    switch (mDisplacementMode) {
        case DISPLACE_AUDIO:
             mAudioFbo.bindTexture(0, 0);
            break;
            
        case DISPLACE_NOISE:
            mVtfFbo.bindTexture( 0, 0 );
            break;
            
        default:
            break;
    }
   
    if(mEnableShadow) {
        // render the shadow map and bind it to texture unit 1,
        // so the shader can access it
        renderShadowMap();
        mDepthFbo.bindDepthTexture(2);
    }
    
    if (mEnableTexture) {
        if(mTexture) mTexture.bind(1);
    }
    
	// Bind and configure displacement shader
	mShaderVtf.bind();
	mShaderVtf.uniform( "displacement", 0 );
    mShaderVtf.uniform( "tex", 1 );
	mShaderVtf.uniform( "eyePoint", getCamera().getEyePoint() );
	mShaderVtf.uniform( "height", mDisplacementHeight );
	mShaderVtf.uniform( "lightingEnabled", mEnableLight );
	mShaderVtf.uniform( "scale", Vec3f(1.0f,1.0f,1.0f) );
	mShaderVtf.uniform( "textureEnabled", mEnableTexture );
	mShaderVtf.uniform( "shadowEnabled", mEnableShadow );
	mShaderVtf.uniform( "flat", mDrawFlatShaded );
	mShaderVtf.uniform( "flatFirstPass", true );
    mShaderVtf.uniform( "faceAlpha", mMeshFaceAlpha );
    mShaderVtf.uniform( "lineAlpha", mMeshLineAlpha );
    if(mEnableShadow) {
        mShaderVtf.uniform( "shadowtex", 2 );
        mShaderVtf.uniform( "flat", mDrawFlatShaded );
        mShaderVtf.uniform( "shadowMatrix", mShadowMatrix );
    }
    
    if ( mDrawWireframe ) {
        if (mDrawFlatShaded) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // color set in shader
        } else {
            glLineWidth(1.0f);
            gl::enableWireframe();
        }
	}
    
    //if (mMeshType == MESHTYPE_CYLINDER) {
        //gl::scale( 1.0f, 200.0f, 1.0f );
    //}

	// Draw mesh
    gl::pushMatrices();
    float scale = 1.0f;
    if (mMeshType == MESHTYPE_CYLINDER) {
        scale = 200.0f;
        if (mCurMesh == 1) {
            mShaderVtf.uniform( "lineAlpha", mMeshLineAlpha*0.5f );
            gl::scale( 1.0f, -scale, 1.0f );
        } else {
            gl::scale( 1.0f, scale, 1.0f );
        }
    } else if (mMeshType == MESHTYPE_TORUS) {
        scale = mScale;
        gl::scale( scale, scale, scale );
    }
	gl::draw( mVboMesh[mCurMesh] );
    
    if (mMeshType == MESHTYPE_FLAT && mDrawMirror) {
        mShaderVtf.uniform( "faceAlpha", mMeshFaceAlpha*0.5f );
        mShaderVtf.uniform( "lineAlpha", mMeshLineAlpha*0.5f );
    }
    if (mDrawMirror || mMeshType == MESHTYPE_CYLINDER) {
        gl::translate(0.0f, scale, 0.0f);
        if (mCurMesh == 0) {
            gl::scale( 1.0f, -scale, 1.0f );
        } else {
            mShaderVtf.uniform( "lineAlpha", mMeshLineAlpha*0.5f );
        }
        gl::draw( mVboMesh[mCurMesh] );
    }
    gl::popMatrices();
    
    if (mDrawWireframe) {
        if (mDrawFlatShaded) {
            mShaderVtf.uniform( "flatFirstPass", false );
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.0f);
            
            // Draw mesh
            mShaderVtf.uniform( "faceAlpha", mMeshFaceAlpha );
            mShaderVtf.uniform( "lineAlpha", mMeshLineAlpha );
            if (mMeshType == MESHTYPE_TORUS) {
                gl::scale( scale, scale, scale );
            }
            gl::draw( mVboMesh[mCurMesh] );
            
            if (mMeshType == MESHTYPE_FLAT && mDrawMirror) {
                mShaderVtf.uniform( "faceAlpha", mMeshFaceAlpha*0.5f );
                mShaderVtf.uniform( "lineAlpha", mMeshLineAlpha*0.5f );
            }
            if (mDrawMirror || mMeshType == MESHTYPE_CYLINDER) {
                gl::translate(0.0f, 1.0f, 0.0f);
                gl::scale( 1.0f, -1.0f, 1.0f );
                gl::draw( mVboMesh[mCurMesh] );
            }
            
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            gl::disableWireframe();
        }
    }
    
	// Unbind everything
    if (mEnableTexture && mTexture) {
        mTexture.unbind();
    }
    if(mEnableShadow) {
        mDepthFbo.unbindTexture();
    }
	mShaderVtf.unbind();
	mVtfFbo.unbindTexture();

	gl::disable( GL_TEXTURE_2D );
}

void Terrain::drawDebug()
{
    //gl::drawCoordinateFrame(15.0f, 2.5f, 1.0f);
    
    if(mDrawShadowMap)
	{
		mDepthFbo.getDepthTexture().enableAndBind();
        
		// we'd like to draw the depth map as a normal texture,
		// so let's temporarily disable the texture compare mode
		glPushAttrib( GL_TEXTURE_BIT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE );
		glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE );
        
		// optional: invert colors when drawing, so black is far away and white is nearby
		//glEnable( GL_BLEND );
		//glBlendFuncSeparate( GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_DST_ALPHA );
		
		gl::color( Color::white() );
		gl::drawSolidRect( Rectf( 0, 256, 256, 0 ), false );
        
		//glDisable( GL_BLEND );
        
		glPopAttrib();
        
		mDepthFbo.unbindTexture();
	}
    
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    Rectf preview( 100.0f, mApp->getWindowHeight() - 200.0f, 180.0f, mApp->getWindowHeight() - 120.0f );
    gl::draw( mVtfFbo.getTexture(), mVtfFbo.getBounds(), preview );
    
    mAudioFbo.bindTexture();
    //TODO: make utility func for making rects with origin/size
    gl::drawSolidRect( Rectf( 100.0f, mApp->getWindowHeight() - 120.0f, 180.0f, mApp->getWindowHeight() - 40.0f ) );
    
    gl::disable( GL_TEXTURE_2D );
}

const Camera& Terrain::getCamera()
{
    switch( mCamType )
    {
        case CAM_STATIC:
            return mStaticCam;
            
        case CAM_SPLINE:
            return mSplineCam.getCamera();
            
        case CAM_TUNNEL:
            return mTunnelCam.getCamera();
            
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

void Terrain::enableLights()
{
	// enable lighting
	gl::enable( GL_LIGHTING );
    
    if (mEnableShadow) {
        mLight->disable();
        
        // setup light 0
        gl::Light light(gl::Light::POINT, 0);
        
        light.lookAt( mLightPosition, Vec3f( 50.0f, 0.0f, 50.0f ) );
        light.setAmbient( Color( 0.0f, 0.0f, 0.0f ) );
        light.setDiffuse( Color( 1.0f, 1.0f, 1.0f ) );
        light.setSpecular( Color( 1.0f, 1.0f, 1.0f ) );
        light.setShadowParams( 60.0f, 50.0f, 300.0f );
        light.enable();
        
        // because I chose to redefine the light every frame,
        // the easiest way to access the light's shadow settings
        // is to store them in a few member variables
        mShadowMatrix = light.getShadowTransformationMatrix( getCamera() );
        mShadowCamera = light.getShadowCamera();
    } else {
        mLight->enable();
    }
}

void Terrain::disableLights()
{
	gl::disable( GL_LIGHTING );
}

void Terrain::setupShadowMap()
{
	static const int size = 2048;
    
	// create a frame buffer object (FBO) containing only a depth buffer
	mDepthFbo = gl::Fbo( size, size, false, false, true );
    
	// set it up for shadow mapping
	mDepthFbo.bindDepthTexture();
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
}

void Terrain::renderShadowMap()
{
	// store the current OpenGL state,
	// so we can restore it when done
	glPushAttrib( GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT );
    
	// bind the shadow map FBO
	mDepthFbo.bindFramebuffer();
    
	// set the viewport to the correct dimensions and clear the FBO
	glViewport( 0, 0, mDepthFbo.getWidth(), mDepthFbo.getHeight() );
	glClear( GL_DEPTH_BUFFER_BIT );
    
	// to reduce artefacts, offset the polygons a bit
	glPolygonOffset( 2.0f, 5.0f );
	glEnable( GL_POLYGON_OFFSET_FILL );
    
	// render the mesh
	gl::enableDepthWrite();
    
	gl::pushMatrices();
    gl::setMatrices( mShadowCamera );
    gl::draw( mVboMesh[mCurMesh] );
	gl::popMatrices();
    
	// unbind the FBO and restore the OpenGL state
	mDepthFbo.unbindFramebuffer();
    
	glPopAttrib();
}

#pragma mark - Mesh

bool Terrain::setupMesh()
{
    // setup for tube
    
    if (mMeshType == MESHTYPE_FLAT) {
        
        // perlin noise generator (see below)
        Perlin	perlin( 3, clock() & 65535 );
        
        // clear the mesh
        mTriMesh.clear();
        
        // create the vertices and texture coords
        size_t width = 256;
        size_t depth = 256;
        
        for(size_t z=0;z<=depth;++z) {
            for(size_t x=0;x<=width;++x) {
                float y = 0.0f;
                
                switch( mMeshType ) {
                    case MESHTYPE_FLAT:
                        y = 0.0f;
                        break;
//                    case MESHTYPE_RANDOM:
//                        //	1. random bumps
//                        y = 5.0f * Rand::randFloat();
//                        break;
//                    case MESHTYPE_SMOOTH:
//                        //	2. smooth bumps (egg container)
//                        y = 5.0f * sinf( (float) M_PI * 0.05f * x ) * cosf( (float) M_PI * 0.05f * z );
//                        break;
//                    case MESHTYPE_PERLIN:
//                        //	3. perlin noise
//                        y = 20.0f * perlin.fBm( Vec3f( static_cast<float>(x), static_cast<float>(z), 0.0f ) * 0.02f );
//                        break;
                }
                
                mTriMesh.appendVertex( Vec3f( static_cast<float>(x), y, static_cast<float>(z) ) );
                mTriMesh.appendTexCoord( Vec2f( static_cast<float>(x) / width, static_cast<float>(z) / depth ) );
            }
        }
        
        // next, create the index buffer
        std::vector<uint32_t>	indices;
        
        for(size_t z=0;z<depth;++z) {
            size_t base = z * (width + 1);
            
            for(size_t x=0;x<width;++x) {
                indices.push_back( base + x );
                indices.push_back( base + x + width + 1 );
                indices.push_back( base + x + 1 );
                
                indices.push_back( base + x + 1 );
                indices.push_back( base + x + width + 1 );
                indices.push_back( base + x + width + 2 );
            }
        }
        
        mTriMesh.appendIndices( &indices.front(), indices.size() );
        
        // use this custom function to create the normal buffer
        mTriMesh.generateNormals();
    }
    else if (mMeshType == MESHTYPE_TUBE)
    {
        // Make the b-spline
//        int degree = 3;
//        bool loop = true;
//        bool open = false;
//        mBSpline = BSpline3f( mCurPoints, degree, loop, open );
        
        // Tube
        std::vector<Vec3f> prof;
        makeCircleProfile( prof, 10.0f, 16 );
        mTube.setProfile( prof );
        mTube.setBSpline( mSplineCam.getSpline() );
        mTube.setNumSegments( 256 );
        mTube.sampleCurve();
        if( 1 ) {//mParallelTransport ) {
            mTube.buildPTF();
        }
        else {
            mTube.buildFrenet();
        }
        mTube.buildMesh( &mTriMesh );
    }
    else if (mMeshType == MESHTYPE_TORUS)
    {
        mTriMesh = MeshHelper::createTorus( Vec2i(256,256), 0.10f );
    }
    else {
        mTriMesh = MeshHelper::createCylinder( Vec2i(32,256), 1.0f, 1.0f, false, false );
    }
    
    mCurMesh = 0;
    mNextMesh = 1;
    
    mVboMesh[mCurMesh] = gl::VboMesh( mTriMesh );
    mVboMesh[mNextMesh] = gl::VboMesh( mTriMesh );
    
    return false;
}

#pragma mark VTF

void Terrain::setupDynamicTexture()
{
    // Params
    mDisplacementHeight	= 8.0f;
	mDisplacementSpeed  = 1.0f;
	mTheta				= 0.0f;
    
    // Load shaders
	try {
		mShaderTex = gl::GlslProg( loadResource( RES_SHADER_TEX_VERT ), loadResource( RES_SHADER_TEX_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile texture shader:\n" << ex.what() << "\n";
	}
	try {
		mShaderVtf = gl::GlslProg( loadResource( RES_SHADER_VTF_VERT ), loadResource( RES_SHADER_VTF_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile VTF shader:\n" << ex.what() << "\n";
	}
    
    //mPerlin = Perlin( 3, clock() & 65535 );
}
