/*
 *  ShaderTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"//TODO: fix this dependency
#include "ShaderTest.h"
#include "Resources.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"

#include "Contour.h"

using namespace ci;

ShaderTest::ShaderTest()
: Scene("shadertest")
{
    mAudioInputHandler.setup(this, true);
}

ShaderTest::~ShaderTest()
{
}

void ShaderTest::setup()
{
    Scene::setup();
    
    Vec2f viewportSize( mApp->getViewportWidth(), mApp->getViewportHeight() );
    mMotionBlurRenderer.setup( viewportSize, boost::bind( &ShaderTest::drawScene, this ) );
    mGridRenderer.setup( viewportSize, boost::bind( &ShaderTest::drawScene, this ) );
    
    mMotionBlur = false;
    mGrid       = false;
    
    setupShaders();
    
    
    // OSC TEST
    //mApp->getOscServer().registerCallback( "/multi/1", this, &ShaderTest::handleOscMessage );
    
}

void ShaderTest::setupShaders()
{
    mShaderType = SHADER_SIMPLICITY;
    
    try
    {
        gl::GlslProg shader;
        
        // PERLIN NOISE
        shader = gl::GlslProg( loadResource( RES_SHADER_CT_TEX_VERT ), loadResource( RES_SHADER_CT_TEX_FRAG ) );
        mShaders.push_back(shader);
        
        mNoiseParams.mNoiseScale            = Vec3f(1.0f,1.0f,0.25f);
        mNoiseParams.mDisplacementSpeed     = 1.0f;
        
        // SIMPLICITY
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_SIMPLICITY_FRAG ) );
        mShaders.push_back(shader);
        
        // MENGER
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_MENGER_FRAG ) );
        mShaders.push_back(shader);
        
        mRedPower = 2;
        mGreenPower = 1;
        mBluePower = 0;
        mColorScale = Vec3f(1.8f, 1.4f, 1.0f);
        mStrengthFactor = 0.03f;
        mStrengthMin = 7.0f;
        mStrengthConst = 4373.11f;
        mIterations = 32;
        mAccumPower = 2.3f;
        mMagnitude = Vec3f(-0.5f, -0.4f, -1.5f);
        mFieldScale = 5.0f;
        mFieldSubtract = 0.7f;
        mTimeScale = 1.0f;
        mPanSpeed = Vec3f(0.0625f, 0.0833f, 0.0078f);
        mUVOffset = Vec3f(1.0f, -1.3f, 0.0f);
        mUVScale = 0.25f;
        
	}
	catch( gl::GlslProgCompileExc &exc )
    {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
	catch( ... )
    {
		std::cout << "Unable to load shader" << std::endl;
	}
}

void ShaderTest::setupInterface()
{
    vector<string> shaderNames;
#define SHADERS_ENTRY( nam, enm ) \
shaderNames.push_back(nam);
    SHADERS_TUPLE
#undef  SHADERS_ENTRY
    mInterface->addEnum(CreateEnumParam( "Shader", (int*)(&mShaderType) )
                        .maxValue(SHADERS_COUNT)
                        .oscReceiver(getName(), "shader")
                        .isVertical(), shaderNames);
    
    mInterface->addParam(CreateBoolParam( "Motion Blur", &mMotionBlur ));
    mInterface->addParam(CreateBoolParam( "Grid Render", &mGrid ));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Noise");
    mInterface->addParam(CreateFloatParam( "noise/speed", &mNoiseParams.mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise/noise", &mNoiseParams.mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,5.0f))
                         .oscReceiver(mName));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Simplicity");
    mInterface->addParam(CreateIntParam( "Red Power", &mRedPower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver("/1/fader2"));
    mInterface->addParam(CreateIntParam( "Green Power", &mGreenPower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver("/1/fader2"));
    mInterface->addParam(CreateIntParam( "Blue Power", &mBluePower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver("/1/fader2"));
    mInterface->addParam(CreateIntParam( "Iterations", &mIterations )
                         .minValue(0)
                         .maxValue(64));
    mInterface->addParam(CreateFloatParam( "StrengthFactor", &mStrengthFactor )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    mInterface->addParam(CreateFloatParam( "StrengthMin", &mStrengthMin )
                         .minValue(0.0f)
                         .maxValue(20.0f));
    mInterface->addParam(CreateFloatParam( "StrengthConst", &mStrengthConst )
                         .minValue(4000.0f)
                         .maxValue(5000.0f));
    mInterface->addParam(CreateFloatParam( "AccumPower", &mAccumPower )
                         .minValue(1.0f)
                         .maxValue(4.0f));
    mInterface->addParam(CreateFloatParam( "FieldScale", &mFieldScale )
                         .minValue(1.0f)
                         .maxValue(10.0f));
    mInterface->addParam(CreateFloatParam( "FieldSubtract", &mFieldSubtract )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateVec3fParam("magnitude", &mMagnitude, Vec3f(-1.0f,-1.0f,-2.0f), Vec3f::zero()));
    mInterface->addParam(CreateVec3fParam("color_scale", &mColorScale, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("PanSpeed", &mPanSpeed, Vec3f::zero(), Vec3f(0.1f,0.1f,0.1f)));
    mInterface->addParam(CreateVec3fParam("UVOffset", &mUVOffset, Vec3f(-4.0f,-4.0f,-4.0f), Vec3f(4.0f,4.0f,4.0f)));
    mInterface->addParam(CreateFloatParam( "UVScale", &mUVScale )
                         .minValue(0.01f)
                         .maxValue(4.0f));
    
    mAudioInputHandler.setupInterface(mInterface);

    mRadius = 100.0f;
}

void ShaderTest::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    if (mGrid)
    {
        mGridRenderer.preDraw();
    }
    else if (mMotionBlur)
    {
        mMotionBlurRenderer.preDraw();
    }
}

void ShaderTest::draw()
{
    gl::pushMatrices();

    if( mGrid )
    {
        mGridRenderer.draw();
    }
    else if( mMotionBlur )
    {
        mMotionBlurRenderer.draw();
    }
    else
    {
        drawScene();
    }
    
    gl::popMatrices();
}

void ShaderTest::shaderPreDraw()
{
    //mAudioInputHandler.getFbo().bindTexture(1);
    Scene* contourScene = mApp->getScene("contour");
    
    if( contourScene && contourScene->isRunning() )
    {
        Contour* contour = static_cast<Contour*>(contourScene);
        contour->getVtfFbo().bindTexture(1);
    }
    else
    {
        mAudioInputHandler.getFbo().bindTexture(1);
    }
    
    gl::GlslProg shader = mShaders[mShaderType];
    shader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    switch( mShaderType )
    {
        case SHADER_NOISE:
            shader.uniform( "theta", (float)(mApp->getElapsedSeconds() * mNoiseParams.mDisplacementSpeed) );
            shader.uniform( "scale", mNoiseParams.mNoiseScale );
            shader.uniform( "color", mColorScale );
            break;
            
        case SHADER_SIMPLICITY:
            shader.uniform( "colorScale", mColorScale );
            shader.uniform( "rPower", mRedPower );
            shader.uniform( "gPower", mGreenPower );
            shader.uniform( "bPower", mBluePower );
            shader.uniform( "strengthFactor", mStrengthFactor );
            shader.uniform( "strengthMin", mStrengthMin );
            shader.uniform( "strengthConst", mStrengthConst );
            shader.uniform( "iterations", mIterations );
            shader.uniform( "accumPower", mAccumPower );
            shader.uniform( "magnitude", mMagnitude );
            shader.uniform( "fieldScale", mFieldScale );
            shader.uniform( "fieldSubtract", mFieldSubtract );
            shader.uniform( "timeScale", mTimeScale );
            shader.uniform( "panSpeed", mPanSpeed );
            shader.uniform( "uvOffset", mUVOffset );
            shader.uniform( "uvScale", mUVScale );
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
            break;
        case SHADER_MENGER:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
            break;
            
        default:
            break;
    }
}

void ShaderTest::drawShaderOutput()
{
    // Draw shader output
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::begin( GL_TRIANGLES );
    
    // Define quad vertices
    const Area& bounds = mApp->getViewportBounds();
    
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
    gl::disable( GL_TEXTURE_2D );
}

void ShaderTest::shaderPostDraw()
{
    gl::GlslProg shader = mShaders[mShaderType];
    shader.unbind();
    
    mAudioInputHandler.getFbo().unbindTexture();
}

void ShaderTest::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    drawShaderOutput();
    
    shaderPostDraw();
    
    gl::popMatrices();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::disableAlphaBlending();
}

void ShaderTest::drawDebug()
{
    mAudioInputHandler.drawDebug(mApp->getViewportSize());
}

//void ShaderTest::handleOscMessage( const ci::osc::Message& message )
//{
//    if( message.getNumArgs() == 2 ) 
//    {
//        if( osc::TYPE_FLOAT == message.getArgType(0) )
//        {
//            mPos.x = message.getArgAsFloat(0) * mApp->getViewportWidth();
//        }
//        if( osc::TYPE_FLOAT == message.getArgType(1) )
//        {
//            mPos.y = message.getArgAsFloat(1) * mApp->getViewportHeight();
//        }        
//        console() << "[osc test] x: " << mPos.x << " y: " << mPos.y << std::endl;
//    }
//}
