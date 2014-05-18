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
#include "Utils.h"

#include "Contour.h"

using namespace ci;
using namespace ci::app;

ShaderTest::ShaderTest()
: Scene("shadertest")
{
    //mAudioInputHandler.setup(this, true);
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
    
    mDynamicTexture.setup(512, 512);
    
    mTexture[0] = gl::Texture( loadImage( loadResource( "gaussian_noise_256_3c.png" ) ) );
    mTexture[1] = gl::Texture( loadImage( loadResource( "colortex2.jpg" ) ) );
    mTexture[2] = gl::Texture( loadImage( loadResource( RES_COLORTEX3 ) ) );
    //mTexture[3] = gl::Texture( loadImage( loadResource( RES_COLORTEX4 ) ) );
    mTexture[3] = mDynamicTexture.getTexture();
    mTextureIndex = 0;
    // OSC TEST
    //mApp->getOscServer().registerCallback( "/multi/1", this, &ShaderTest::handleOscMessage );
    
    mStripesParams.mTimeScale = 0.1f;
}

void ShaderTest::reset()
{
    mTimeController.reset();
}

void ShaderTest::setupShaders()
{
    mShaderType = SHADER_TEST;
    
#define SHADERS_ENTRY( nam, glsl, enm ) \
mShaders.push_back( loadFragShader( glsl ) );
    SHADERS_TUPLE
#undef SHADERS_ENTRY
}

void ShaderTest::setupInterface()
{
    vector<string> shaderNames;
#define SHADERS_ENTRY( nam, glsl, enm ) \
shaderNames.push_back(nam);
    SHADERS_TUPLE
#undef  SHADERS_ENTRY
    mInterface->addEnum(CreateEnumParam( "Shader", (int*)(&mShaderType) )
                        .maxValue(SHADERS_COUNT)
                        .oscReceiver(getName(), "shader")
                        .isVertical(), shaderNames);
    
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->addParam(CreateBoolParam( "Motion Blur", &mMotionBlur ));
    mInterface->addParam(CreateBoolParam( "Grid Render", &mGrid ));
    
    mInterface->addParam(CreateIntParam("texture", &mTextureIndex)
                         .maxValue(MAX_TEXTURES-1)
                         .oscReceiver(getName()));
    
    // SHADER_STRIPES
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Stripes");
    mInterface->addParam(CreateFloatParam("stripes/timescale", &mStripesParams.mTimeScale)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("stripes/countscale", &mStripesParams.mCountScale)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("stripes/countscalemult", &mStripesParams.mCountScale)
                         .minValue(1.0f)
                         .maxValue(1000.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateColorParam("stripes/color1", &mStripesParams.mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("stripes/color2", &mStripesParams.mColor2, kMinColor, kMaxColor));
//
//    mInterface->gui()->addColumn();
//    mInterface->gui()->addLabel("After Effect");
//    mInterface->addParam(CreateIntParam("aftereffect/pattern", &mAfterEffectParams.mEffect)
//                         .maxValue(2)
//                         .oscReceiver(getName()));
    
    mDynamicTexture.setupInterface(mInterface, mName);
    
    
   //mAudioInputHandler.setupInterface(mInterface);

    mRadius = 100.0f;
}

void ShaderTest::update(double dt)
{
    Scene::update(dt);
    
    //mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    if (mGrid)
    {
        mGridRenderer.preDraw();
    }
    else if (mMotionBlur)
    {
        mMotionBlurRenderer.preDraw();
    }
    
    mDynamicTexture.update(dt);
    
    mTimeController.update(dt);
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
        if( mApp->getAudioInputHandler().hasTexture() )
        {
            mApp->getAudioInputHandler().getFbo().bindTexture(1);
        }
    }
    
    mTexture[mTextureIndex].bind(0);
    
    gl::GlslProg shader = mShaders[mShaderType];
    shader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    switch( mShaderType )
    {
        case SHADER_STRIPES:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "timeScale", mStripesParams.mTimeScale );
            shader.uniform( "color1", mStripesParams.mColor1 );
            shader.uniform( "color2", mStripesParams.mColor2 );
            shader.uniform( "countScale", mStripesParams.mCountScale );
            break;
        
        default:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "iChannelTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "iChannel0", 0 );
            shader.uniform( "iChannel1", 1 );
            shader.uniform( "iMouse", Vec2f::one()*0.5f );
            break;
    }
}

void ShaderTest::shaderPostDraw()
{
    gl::GlslProg shader = mShaders[mShaderType];
    shader.unbind();
    
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
    mTexture[mTextureIndex].unbind();
}

void ShaderTest::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    Utils::drawTexturedRect( mApp->getViewportBounds() );
    
    shaderPostDraw();
    
    gl::popMatrices();
}

void ShaderTest::drawDebug()
{
    mApp->getAudioInputHandler().drawDebug(mApp->getViewportSize());

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
