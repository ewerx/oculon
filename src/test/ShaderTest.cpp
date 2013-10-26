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
    
    mTexture[0] = gl::Texture( loadImage( loadResource( "colortex1.jpg" ) ) );
    mTexture[1] = gl::Texture( loadImage( loadResource( RES_COLORTEX2 ) ) );
    mTexture[2] = gl::Texture( loadImage( loadResource( RES_COLORTEX3 ) ) );
    mTexture[3] = gl::Texture( loadImage( loadResource( RES_COLORTEX4 ) ) );
    // OSC TEST
    //mApp->getOscServer().registerCallback( "/multi/1", this, &ShaderTest::handleOscMessage );
    mElapsedTime = 0.0f;
}

void ShaderTest::reset()
{
    mElapsedTime = 0.0f;
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
        mNoiseParams.mLevels                = 64.0f;
        mNoiseParams.mEdgeThickness         = 0.0f;
        mNoiseParams.mBrightness            = 1.0f;
        
        // SIMPLICITY
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_SIMPLICITY_FRAG ) );
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
        
        // too slow!
        // MENGER
//        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_MENGER_FRAG ) );
//        mShaders.push_back(shader);
        
        // KALI
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_PAINT_FRAG ) );
        mShaders.push_back(shader);
        
        mKaliParams.iterations=20;
        mKaliParams.scale=1.35f;
        mKaliParams.fold= Vec2f(0.5f,0.5f);
        mKaliParams.translate= Vec2f(1.5f,1.5f);
        mKaliParams.zoom=0.17f;
        mKaliParams.brightness=7.f;
        mKaliParams.saturation=0.2f;
        mKaliParams.texturescale=0.15f;
        
        mKaliParams.rotspeed=0.005f;
        
        mKaliParams.colspeed=0.05f;
        
        mKaliParams.antialias=2.f;
        mTextureIndex = 0;
    
        // VORONOI
        shader = gl::GlslProg( loadResource( RES_PASSTHRU2_VERT ), loadResource( RES_SHADER_VORONOI_FRAG ) );
        mShaders.push_back(shader);
        
        mVoronoiParams.mBorderColor = Vec3f( 1.0f, 0.6f, 0.1f );
        mVoronoiParams.mZoom = 8.0f;
        mVoronoiParams.mBorderIn = 0.04f;
        mVoronoiParams.mBorderOut = 0.07f;
        mVoronoiParams.mSeedColor = Vec3f( 1.0f, 0.6f, 0.1f );
        mVoronoiParams.mSeedSize = 0.12f;
        mVoronoiParams.mCellLayers = 8.0f;
        mVoronoiParams.mCellBrightness = 0.5f;
        mVoronoiParams.mCellBorderStrength = 0.5f;
        mVoronoiParams.mCellColor = Vec3f( 1.0f, 1.0f, 1.0f );
        mVoronoiParams.mSpeed = 1.0f;
        mVoronoiParams.mDistortion = 1.0f;
        
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

        // CIRCLEWAVE
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "circlewave_frag.glsl" ) );
        mShaders.push_back(shader);
        
        // CELLS
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "cells_frag.glsl" ) );
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
        
        // SHADER_MAINSEQUENCE
        shader = gl::GlslProg( loadResource( "passThru.vert" ), loadResource( "mainsequence_frag.glsl" ) );
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
    
    mInterface->addParam(CreateBoolParam( "Motion Blur", &mMotionBlur ));
    mInterface->addParam(CreateBoolParam( "Grid Render", &mGrid ));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Noise");
    mInterface->addParam(CreateFloatParam( "noise/speed", &mNoiseParams.mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise/noise", &mNoiseParams.mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,5.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "levels", &mNoiseParams.mLevels )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "brightness", &mNoiseParams.mBrightness )
                         .oscReceiver(getName())
                         .midiInput(0,2,16));

    mInterface->addParam(CreateFloatParam( "edgeThickness", &mNoiseParams.mEdgeThickness )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    
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
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Kali");
    mInterface->addParam(CreateIntParam( "texture", &mTextureIndex )
                         .maxValue(MAX_TEXTURES-1)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "kali/iterations", &mKaliParams.iterations )
                         .maxValue(64)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec2fParam("kali/fold", &mKaliParams.fold, Vec2f::zero(), Vec2f(1.0f,1.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateVec2fParam("kali/translate", &mKaliParams.translate, Vec2f::zero(), Vec2f(5.0f,5.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "kali/scale", &mKaliParams.scale )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/zoom", &mKaliParams.zoom )
                         .maxValue(1.0f)
                         .oscReceiver(getName())
                         .midiInput(0, 2, 19));
    mInterface->addParam(CreateFloatParam( "kali/brightness", &mKaliParams.brightness )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/saturation", &mKaliParams.saturation )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/texturescale", &mKaliParams.texturescale )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/rotspeed", &mKaliParams.rotspeed )
                         .maxValue(0.1f)
                         .oscReceiver(getName())
                         .midiInput(0, 2, 20));
    mInterface->addParam(CreateFloatParam( "kali/colspeed", &mKaliParams.colspeed )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/antialias", &mKaliParams.antialias )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Voronoi");
    mInterface->addParam(CreateFloatParam("voronoi/speed", &mVoronoiParams.mSpeed)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/zoom", &mVoronoiParams.mZoom)
                         .maxValue(256.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/distortion", &mVoronoiParams.mDistortion)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateVec3fParam("voronoi/line_color", &mVoronoiParams.mBorderColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateFloatParam("voronoi/borderin", &mVoronoiParams.mBorderIn)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/borderout", &mVoronoiParams.mBorderOut)
                         .oscReceiver(getName()));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateVec3fParam("voronoi/seed_color", &mVoronoiParams.mSeedColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateFloatParam("voronoi/seedsize", &mVoronoiParams.mSeedSize)
                         .oscReceiver(getName()));
    mInterface->gui()->addSeparator();
    mInterface->addParam(CreateVec3fParam("voronoi/cell_color", &mVoronoiParams.mCellColor, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateFloatParam("voronoi/cell_brightness", &mVoronoiParams.mCellBrightness)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("voronoi/cell_strength", &mVoronoiParams.mCellBorderStrength)
                         .oscReceiver(getName()));
    
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
    
    mElapsedTime += dt;
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
        if( mAudioInputHandler.hasTexture() )
        {
            mAudioInputHandler.getFbo().bindTexture(1);
        }
    }
    
    mTexture[mTextureIndex].bind(0);
    
    gl::GlslProg shader = mShaders[mShaderType];
    shader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    switch( mShaderType )
    {
        case SHADER_NOISE:
            shader.uniform( "theta", (float)(mElapsedTime * mNoiseParams.mDisplacementSpeed) );
            shader.uniform( "scale", mNoiseParams.mNoiseScale );
            shader.uniform( "colorScale", mColorScale );
            shader.uniform( "alpha", mNoiseParams.mBrightness );
            shader.uniform( "levels", mNoiseParams.mLevels );
            shader.uniform( "edgeThickness", mNoiseParams.mEdgeThickness );
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
            shader.uniform( "iGlobalTime", (float)mElapsedTime );
            break;
        case SHADER_PAINT:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mElapsedTime );
            shader.uniform( "iChannel0", 0 );
            shader.uniform( "iterations", mKaliParams.iterations );
            shader.uniform( "scale", mKaliParams.scale );
            shader.uniform( "fold", mKaliParams.fold );
            shader.uniform( "translate", mKaliParams.translate );
            shader.uniform( "zoom", mKaliParams.zoom );
            shader.uniform( "brightness", mKaliParams.brightness );
            shader.uniform( "saturation", mKaliParams.saturation );
            shader.uniform( "texturescale", mKaliParams.texturescale );
            shader.uniform( "rotspeed", mKaliParams.rotspeed );
            shader.uniform( "colspeed", mKaliParams.colspeed );
            shader.uniform( "antialias", mKaliParams.antialias );
            break;
            
        case SHADER_VORONOI:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
            shader.uniform( "borderColor", mVoronoiParams.mBorderColor );
            shader.uniform( "zoom", mVoronoiParams.mZoom );
            shader.uniform( "speed", mVoronoiParams.mSpeed );
            shader.uniform( "borderIn", mVoronoiParams.mBorderIn );
            shader.uniform( "borderOut", mVoronoiParams.mBorderOut );
            shader.uniform( "seedSize", mVoronoiParams.mSeedSize );
            shader.uniform( "seedColor", mVoronoiParams.mSeedColor );
            shader.uniform( "cellLayers", mVoronoiParams.mCellLayers );
            shader.uniform( "cellColor", mVoronoiParams.mCellColor );
            shader.uniform( "cellBorderStrength", mVoronoiParams.mCellBorderStrength );
            shader.uniform( "cellBrightness", mVoronoiParams.mCellBrightness );
            shader.uniform( "distortion", mVoronoiParams.mDistortion );
            break;
            
        case SHADER_STRIPES:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
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
            shader.uniform( "iMouse", mKaliParams.translate );
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
            break;
            
        case SHADER_INTERSTELLAR:
        case SHADER_CIRCLEWAVE:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mElapsedTime );
            shader.uniform( "iChannel0", 1 );
            break;
            
        default:
            shader.uniform( "iResolution", resolution );
            shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
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
    
    if( mAudioInputHandler.hasTexture() )
    {
        mAudioInputHandler.getFbo().unbindTexture();
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
