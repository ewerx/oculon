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
#include "cinder/CinderMath.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"
#include <vector>

using namespace ci;
using namespace std;


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
    mCamType = CAM_MANUAL;
    mDrawWireframe = true;
	mDrawFlatShaded = false;
	mDrawShadowMap = false;
    mMeshType = MESHTYPE_FLAT;
    mEnableLight = false;
    mEnableShadow = false;
    mNoiseScale = Vec3f(1.0f,1.0f,1.0f);
    
    // Audio
    mFalloff = 2.0f;
    mFalloffMode = FALLOFF_OUTBOUNCE;
    mAudioRowShift = 0;
    mAudioRowShiftTime = 0.0f;
    mAudioRowShiftDelay = 0.25f;
    
    // create the mesh
	setupMesh();
    
//#ifdef STATIC_TERRAIN
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
    
    mSplineCam.setup();
    
    mStaticCam = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.0001f, 100.0f );
	mStaticCam.lookAt( Vec3f( 0.0f, 0.0f, -16.0f ), Vec3f::zero() );
    
    mApp->setCamera(Vec3f( 0.0f, 0.0f, -16.0f ), Vec3f::zero(), Vec3f(0.0f,1.0f,0.0f));
    
    // AUDIO
    mAudioFboDim    = 16; // 256 bands
    mAudioFboSize   = Vec2f( mAudioFboDim, mAudioFboDim );
    mAudioFboBounds = Area( 0, 0, mAudioFboDim, mAudioFboDim );
    gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
    mAudioFbo       = gl::Fbo( mAudioFboDim, mAudioFboDim, format );
    
    Scene::setup();
    reset();
}

// ----------------------------------------------------------------
//
void Terrain::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "wireframe", &mDrawWireframe )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "flat shaded", &mDrawFlatShaded )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "lighting", &mEnableLight )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "shadow", &mEnableShadow )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "disp speed", &mDisplacementSpeed )
                         .maxValue(5.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "disp height", &mDisplacementHeight )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise", &mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,50.0f))
                         .oscReceiver(mName));
//    mInterface->addParam(CreateBoolParam( "shadow map", &mDrawShadowMap )
//                         .oscReceiver(getName()));

    
    mInterface->gui()->addColumn();
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
    mSplineCam.setupInterface(mInterface, mName);
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
    
    if( mCamType == CAM_SPLINE )
        mSplineCam.update(dt);
    
//#ifdef STATIC_TERRAIN
    // animate light
	float x = 50.0f + 150.0f * (float) sin( 0.20 * getElapsedSeconds() );
	float y = 50.0f +  45.0f * (float) cos( 0.13 * getElapsedSeconds() );
	float z = 50.0f + 150.0f * (float) cos( 0.20 * getElapsedSeconds() );
    
	mLightPosition = Vec3f(x, y, z);
    
    //updateMesh();
//#endif
    
    // Update animation position
	float time = (float)getElapsedSeconds() * mDisplacementSpeed;
	mTheta = time;//math<float>::sin( time );
    
    drawDynamicTexture();
}


void Terrain::drawDynamicTexture()
{
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
    
	///////////////////////////////////////////////////////////////
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
    
	Surface32f fftSurface( mAudioFbo.getTexture() );
	Surface32f::Iter it = fftSurface.getIter();
	while( it.line() )
    {
        int32_t index = row * mAudioFboDim;
		while( it.pixel() && index < dataSize )
        {
            if (fftLogData[index].y > mFftFalloff[index])
            {
                mFftFalloff[index] = fftLogData[index].y;
                timeline().apply( &mFftFalloff[index], 0.0f, mFalloff, getFalloffFunction() );
            }
            
			it.r() = mFftFalloff[index];
            it.g() = 0.0f; // UNUSED
			it.b() = 0.0f; // UNUSED
			it.a() = 1.0f; // UNUSED
            
            ++index;
		}
        
        ++row;
        if (row >= mAudioFboDim)
            row = 0;
	}
    
    if (mAudioRowShiftTime >= mAudioRowShiftDelay)
    {
        mAudioRowShiftTime = 0.0f;
        ++mAudioRowShift;
        if (mAudioRowShift >= mAudioFboDim) {
            mAudioRowShift = 0;
        }
    }
	
    glShadeModel( GL_FLAT );
	gl::Texture fftTexture( fftSurface );
	mAudioFbo.bindFramebuffer();
	gl::setMatricesWindow( mAudioFboSize, false );
	gl::setViewport( mAudioFboBounds );
	gl::draw( fftTexture );
	mAudioFbo.unbindFramebuffer();
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

#pragma mark - Render 

// ----------------------------------------------------------------
//
void Terrain::draw()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT );
    
    gl::pushMatrices();
	gl::setMatrices( getCamera() );
    
#ifdef STATIC_TERRAIN
	// remember what OpenGL effects are enabled by default,
	// so we can restore that situation at the end of the draw()
	glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT );
    
	// enable the depth buffer
	gl::enableDepthRead();
	gl::enableDepthWrite();
    
	// draw origin and axes
	//gl::drawCoordinateFrame(15.0f, 2.5f, 1.0f);
    
	// draw light position
	if(mDrawShadowMap) {
		gl::color( Color(1.0f, 1.0f, 0.0f) );
		gl::drawFrustum( mShadowCamera );
	}
    
	// the annoying thing with lights is that they will automatically convert the
	// specified coordinates to world coordinates, which depend on the
	// current camera matrices. So you have to keep telling where the lights are
	// every time the camera moves, otherwise the light will move along
	// with the camera, which causes undesired behavior.
	// To avoid this, and quite a few other annoying things with lights,
	// I usually redefine them every frame, using a function to keep things tidy.
    if(mEnableLight)
        enableLights();
    else
        disableLights();
    
	// render the shadow map and bind it to texture unit 0,
	// so the shader can access it
	renderShadowMap();
	mDepthFbo.bindDepthTexture(0);
    
	// enable texturing and bind texture to texture unit 1
	gl::enable( GL_TEXTURE_2D );
	if(mTexture) mTexture.bind(1);
    
	// bind the shader and set the uniform variables
	if(mShader) {
		mShader.bind();
		mShader.uniform("tex0", 0);
		mShader.uniform("tex1", 1);
		mShader.uniform("flat", mDrawFlatShaded);
		mShader.uniform("shadowMatrix", mShadowMatrix);
	}
	
	// draw the mesh
	if(mDrawWireframe)
		gl::enableWireframe();
    
	gl::color( Color::white() );
	gl::draw( mTriMesh );
    
	if(mDrawWireframe)
		gl::disableWireframe();
    
	// unbind the shader
	if(mShader) mShader.unbind();
    
	// unbind the texture
	if(mTexture) mTexture.unbind();
    
	//
	mDepthFbo.unbindTexture();
#endif
    
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
    
	// Use arcball to rotate model view
//	glMultMatrixf( mArcball.getQuat() );
    
	// Enabled lighting, texture mapping, wireframe
	gl::enable( GL_TEXTURE_2D );
    
    if(mEnableLight) {
        enableLights();
    }
    else {
        disableLights();
    }
    
	// Bind textures
	//mVtfFbo.bindTexture( 0, 0 );
    mAudioFbo.bindTexture(0, 0);
    
    if(mEnableShadow) {
        // render the shadow map and bind it to texture unit 0,
        // so the shader can access it
        renderShadowMap();
        mDepthFbo.bindDepthTexture(1);
    }
    
	// Bind and configure displacement shader
	mShaderVtf.bind();
	mShaderVtf.uniform( "displacement", 0 );
	mShaderVtf.uniform( "eyePoint", getCamera().getEyePoint() );
	mShaderVtf.uniform( "height", mDisplacementHeight );
	mShaderVtf.uniform( "lightingEnabled", (mEnableShadow && mEnableLight) );
	mShaderVtf.uniform( "scale", Vec3f(1.0f,1.0f,1.0f) );
	//mShaderVtf.uniform( "tex", 1 );
	mShaderVtf.uniform( "textureEnabled", false );
    mShaderVtf.uniform( "tex0", 1 );
    mShaderVtf.uniform( "flat", mDrawFlatShaded );
    mShaderVtf.uniform( "shadowMatrix", mShadowMatrix );
    
    if ( mDrawWireframe ) {
		gl::enableWireframe();
	}
    
	// Draw mesh
	gl::draw( mVboMesh );
	
	// Unbind everything
    if(mEnableShadow) {
        mDepthFbo.unbindTexture();
    }
	mShaderVtf.unbind();
	mVtfFbo.unbindTexture();
	
	// Disable wireframe, texture mapping, lighting
	if ( mDrawWireframe ) {
		gl::disableWireframe();
	}

	gl::disable( GL_TEXTURE_2D );
}

void Terrain::drawDebug()
{
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
	// setup light 0
	gl::Light light(gl::Light::POINT, 0);
    
	light.lookAt( mLightPosition, Vec3f( 50.0f, 0.0f, 50.0f ) );
	light.setAmbient( Color( 0.0f, 0.0f, 0.0f ) );
	light.setDiffuse( Color( 1.0f, 1.0f, 1.0f ) );
	light.setSpecular( Color( 1.0f, 1.0f, 1.0f ) );
	light.setShadowParams( 60.0f, 50.0f, 300.0f );
	light.enable();
    
	// enable lighting
	gl::enable( GL_LIGHTING );
    
	// because I chose to redefine the light every frame,
	// the easiest way to access the light's shadow settings
	// is to store them in a few member variables
	mShadowMatrix = light.getShadowTransformationMatrix( getCamera() );
	mShadowCamera = light.getShadowCamera();
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
    gl::draw( mVboMesh );
	gl::popMatrices();
    
	// unbind the FBO and restore the OpenGL state
	mDepthFbo.unbindFramebuffer();
    
	glPopAttrib();
}

#pragma mark - Mesh

void Terrain::setupMesh()
{
//#ifdef STATIC_TERRAIN
	// perlin noise generator (see below)
	Perlin	perlin( 3, clock() & 65535 );
    
	// clear the mesh
	mTriMesh.clear();
    
	// create the vertices and texture coords
	size_t width = 120;
	size_t depth = 120;
    
	for(size_t z=0;z<=depth;++z) {
		for(size_t x=0;x<=width;++x) {
			float y = 0.0f;
            
			switch( mMeshType ) {
                case MESHTYPE_FLAT:
                    y = 0.0f;
                    break;
                case MESHTYPE_RANDOM:
                    //	1. random bumps
                    y = 5.0f * Rand::randFloat();
                    break;
                case MESHTYPE_SMOOTH:
                    //	2. smooth bumps (egg container)
                    y = 5.0f * sinf( (float) M_PI * 0.05f * x ) * cosf( (float) M_PI * 0.05f * z );
                    break;
                case MESHTYPE_PERLIN:
                    //	3. perlin noise
                    y = 20.0f * perlin.fBm( Vec3f( static_cast<float>(x), static_cast<float>(z), 0.0f ) * 0.02f );
                    break;
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
//#endif
    
    // Initialize FBO
	gl::Fbo::Format format;
	format.setColorInternalFormat( GL_RGB32F_ARB );
	mVtfFbo = gl::Fbo( 64, 64, format );
    
	// Initialize FBO texture
	mVtfFbo.bindFramebuffer();
	gl::setViewport( mVtfFbo.getBounds() );
	gl::clear();
	mVtfFbo.unbindFramebuffer();
	mVtfFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
    
    // Generate sphere
	//mVboMesh = gl::VboMesh( MeshHelper::createSphere( Vec2i(64,64) ) );
    //mVboMesh = gl::VboMesh( MeshHelper::createCylinder( Vec2i(32,32), 1.0f, 1.0f, false, false ) );//
    mVboMesh = gl::VboMesh( mTriMesh );
}

void Terrain::updateMesh()
{
    size_t width = 120;
    size_t depth = 120;
    for(size_t x=0;x<=width;++x) {
        mTriMesh.getVertices().pop_back();
        //mTriMesh.getTexCoords().pop_back();
    }
    
    int z = 120;
    for(size_t x=0;x<=width;++x) {
        float y = 5.0f * Rand::randFloat();
        mTriMesh.appendVertex( Vec3f( static_cast<float>(x), y, static_cast<float>(z) ) );
        //mTriMesh.appendTexCoord( Vec2f( static_cast<float>(x) / width, static_cast<float>(z) / depth ) );
    }
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
