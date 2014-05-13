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
    
    mTexture[0] = gl::Texture( loadImage( loadResource( "gaussian_noise_256_3c.png" ) ) );
    mTexture[1] = gl::Texture( loadImage( loadResource( "colortex2.jpg" ) ) );
    mTexture[2] = gl::Texture( loadImage( loadResource( RES_COLORTEX3 ) ) );
    mTexture[3] = gl::Texture( loadImage( loadResource( RES_COLORTEX4 ) ) );
    mTextureIndex = 0;
    // OSC TEST
    //mApp->getOscServer().registerCallback( "/multi/1", this, &ShaderTest::handleOscMessage );
}

void ShaderTest::reset()
{
    mTimeController.reset();
}

void ShaderTest::setupShaders()
{
    mShaderType = SHADER_TEST;
    
    try
    {
        gl::GlslProg shader;
        
        // TEST DU JOUR
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "test_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // too slow!
        // MENGER
//        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_MENGER_FRAG ) );
//        mShaders.push_back(shader);
        
        // FRAGMENT: ERROR: 0:180: Swizzle component 'z' indexes beyond end of input vector (length 2)
        // METAHEXBALLS
//        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_METAHEXBALLS_FRAG ) );
//        mShaders.push_back(shader);
        
        // TRIPPING
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_TRIPPING_FRAG ) );
        mShaders.push_back(shader);
        
        // STRIPES
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_STRIPES_FRAG ) );
        mShaders.push_back(shader);
        
        mStripesParams.mTimeScale = 0.1f;
        //mStripes.mColor1 = ColorAf::white();
        //mStripes.mColor2 = ColorAf::black();
        
        // FLICKER
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_ENERGYFLICKER_FRAG ) );
        mShaders.push_back(shader);
        
        // INVERSION
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_INVERSION_FRAG ) );
        mShaders.push_back(shader);
        
        // CYMATICS -- too slow
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "cymatics_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // INVERSION
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "logistic_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // INTERSTELLAR
        // -- star formation from noise texture not working
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "interstellar_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // STRIPEY
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "stripey_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // SHADER_MOIRE
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "moire_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // SHADER_VOLUMETRICLINES
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "volumetriclines_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // SHADER_SQUARENOISE
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "squarenoise_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // SHADER_AFTEREFFECT
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "aftereffect_frag.glsl" ) );
        mShaders.push_back(shader);
        
        mAfterEffectParams.mEffect = 0;
        
        // SHADER_MAINSEQUENCE
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "mainsequence_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // SHADER_MAINSEQUENCE
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "infinitefall_frag.glsl" ) );
        mShaders.push_back(shader);
        
// NEEDS AUDIO INPUT
/*
         // LIGHTGLOW
         shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_LIGHTGLOW_FRAG ) );
         mShaders.push_back(shader);
*/
        
// TOO SLOW!
/*
        // RASTERIZER
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_RASTERIZER_FRAG ) );
        mShaders.push_back(shader);

         // GLASSFIELD
         shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_GLASSFIELD_FRAG ) );
         mShaders.push_back(shader);

        // POLYCHORA
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_POLYCHORA_FRAG ) );
        mShaders.push_back(shader);
 
         // COSMOS
         shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_COSMOS_FRAG ) );
         mShaders.push_back(shader);
        
         // CLOUD
         shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_CLOUD_FRAG ) );
         mShaders.push_back(shader);
 
*/
        
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
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("After Effect");
    mInterface->addParam(CreateIntParam("aftereffect/pattern", &mAfterEffectParams.mEffect)
                         .maxValue(2)
                         .oscReceiver(getName()));
    
    
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
//        case SHADER_MENGER:
//        case SHADER_POLYCHORA:
//            shader.uniform( "iResolution", resolution );
//            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
//            shader.uniform( "iMouse", mKaliParams.translate );
//            break;

        case SHADER_CYMATICS:
//            shader.uniform( "iMouse", mKaliParams.translate );
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
            break;
            
        case SHADER_INTERSTELLAR:
        case SHADER_MAINSEQUENCE:
        case SHADER_INFINITEFALL:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "iChannel0", 0 );
            shader.uniform( "iChannel1", 1 );
            break;
            
        case SHADER_AFTEREFFECT:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "iPattern", mAfterEffectParams.mEffect );
            
        default:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "iChannelTime", (float)mTimeController.getElapsedSeconds() );
            shader.uniform( "iChannel0", 0 );
            shader.uniform( "iChannel1", 1 );
            shader.uniform( "iMouse", Vec2f::zero() );
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
    
    drawShaderOutput();
    
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
