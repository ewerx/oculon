//
//  ObjectShaders.cpp
//  Oculon
//
//  Created by Ehsan on 13-11-02.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "ObjectShaders.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"
#include "Utils.h"
#include "LissajousShader.h"


using namespace ci;
using namespace ci::app;
using namespace std;

ObjectShaders::ObjectShaders()
: Scene("objectshaders")
{
    //mAudioInputHandler.setup(true);
}

ObjectShaders::~ObjectShaders()
{
}

void ObjectShaders::setup()
{
    Scene::setup();
    
    gl::Fbo::Format format;
    format.enableMipmapping(false);
    format.enableDepthBuffer(false);
    format.setCoverageSamples(8);
    format.setSamples(4); // 4x AA
	//format.setColorInternalFormat( GL_RGB32F_ARB );
    mShaderFbo = gl::Fbo( mApp->getViewportWidth(), mApp->getViewportHeight(), format );
    
    setupShaders();
    
    // color maps
    mColorMapTexture[0] = gl::Texture( loadImage( loadResource( "colortex1.jpg" ) ) );
    mColorMapTexture[1] = gl::Texture( loadImage( loadResource( "colortex2.jpg" ) ) );
    mColorMapTexture[2] = gl::Texture( loadImage( loadResource( "colortex3.jpg" ) ) );
    mColorMapTexture[3] = gl::Texture( loadImage( loadResource( "colortex4.jpg" ) ) );
    mColorMapIndex = 0;
    
    mNoiseTexture = gl::Texture( loadImage( loadResource( "gaussian_noise_256_3c.png" ) ) );
    
    mShaderType = 0;

    mDrawOnSphere = false;
    
    mAudioResponseFreqMin = 0.0f;
    mAudioResponseFreqMax = 1.0f;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mColor3 = ColorA::white();
    
    mBackgroundAlpha = 0.0f;
    
    reset();
}

void ObjectShaders::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new FragShader( "Test",      "test_frag.glsl" ) );
    mShaders.push_back( new LissajousShader() );
    
    // metahex
    mMetaHexParams.mQuality = 4;
    mMetaHexParams.mRenderSteps = 33;
    mMetaHexParams.mLightSpeed = 1.0f;
    mMetaHexParams.mSpeed = 1.0f;
    mMetaHexParams.mNumObjects = 9;
    mMetaHexParams.mCoeffecients = Vec3f(0.967f,0.423f,0.76321f);
    mMetaHexParams.mAudioCoeffs = false;
    mMetaHexParams.mObjTime = 0.0f;
    mMetaHexParams.mLightTime = 0.0f;
    
    // retina
    mRetinaParams.mDialation = 0.2f;
    mRetinaParams.mDialationScale = 1.0f;
    mRetinaParams.mPatternAmp = 0.05f;
    mRetinaParams.mPatternFreq = 20.0f;
    mRetinaParams.mAudioPattern = false;
    mRetinaParams.mScale = 1.0f;
    
    // biofractal
    mBioFractalParams.mIterations = 25;
    mBioFractalParams.mJulia = Vec3f(-2.f,-1.5f,-.5f);
    mBioFractalParams.mRotation = Vec3f(0.5f,-0.05f,-0.5f);
    mBioFractalParams.mLightDir = Vec3f(0.5f,1.f,0.5f);
    mBioFractalParams.mScale = 1.27f;
    mBioFractalParams.mRotAngle = 40.0f;
    mBioFractalParams.mAmplitude = 0.45f;
    mBioFractalParams.mDetail = 0.025f;
    
    
    // fireball
    mFireballParams.mRotationSpeed = 1.0f;
    mFireballParams.mDensity = 1.0f;
}

void ObjectShaders::reset()
{
   mTimeController.reset();
}

void ObjectShaders::setupInterface()
{
    vector<string> shaderNames;
    for( FragShader* shader : mShaders )
    {
        shaderNames.push_back(shader->getName());
    }
    mInterface->addEnum(CreateEnumParam( "shader", (int*)(&mShaderType) )
                        .maxValue(shaderNames.size())
                        .oscReceiver(getName(), "shader")
                        .isVertical(), shaderNames);
    
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color3", &mColor3, kMinColor, kMaxColor));
    
    mInterface->addParam(CreateIntParam( "colormap", &mColorMapIndex )
                         .maxValue(MAX_COLORMAPS-1)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateFloatParam("freqmin", &mAudioResponseFreqMin)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("freqmax", &mAudioResponseFreqMax)
                         .oscReceiver(getName()));
    
    // custom params
    mInterface->gui()->addColumn();
    for( FragShader* shader : mShaders )
    {
        if (shader)
        {
            shader->setupInterface(mInterface, mName);
        }
    }
    
    // TODO: refactor
    // SHADER_METAHEX
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("metahex");
    mInterface->addParam(CreateIntParam( "metahex/objects", &mMetaHexParams.mNumObjects )
                         .maxValue(24)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "metahex/steps", &mMetaHexParams.mRenderSteps )
                         .maxValue(128)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "metahex/quality", &mMetaHexParams.mQuality )
                         .maxValue(16)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "metahex/speed", &mMetaHexParams.mSpeed )
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "metahex/lightspeed", &mMetaHexParams.mLightSpeed )
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("metahex/coeffs", &mMetaHexParams.mCoeffecients, Vec3f::zero(), Vec3f(1.0f,1.0f,1.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("metahex/audiocoeffs", &mMetaHexParams.mAudioCoeffs )
                         .oscReceiver(mName));
    
    // retina
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("retina");
    mInterface->addParam(CreateFloatParam("retina/scale", &mRetinaParams.mScale)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("retina/audiodialation", &mRetinaParams.mAudioDialation )
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/dialation", &mRetinaParams.mDialation)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/dialationscale", &mRetinaParams.mDialationScale)
                         .minValue(1.0f)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/pattamp", &mRetinaParams.mPatternAmp)
                         .minValue(0.001f)
                         .maxValue(0.5f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/pattfreq", &mRetinaParams.mPatternFreq)
                         .minValue(1.0f)
                         .maxValue(40.0f)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateBoolParam("retina/audiopattern", &mRetinaParams.mAudioPattern )
                         .oscReceiver(mName));
    
    // SHADER_BIOFRACTAL
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("biofractal");
    mInterface->addParam(CreateIntParam( "biofractal/iterations", &mBioFractalParams.mIterations )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("biofractal/rotation", mBioFractalParams.mRotation.ptr(), Vec3f(-1.0f,-1.0f,-1.0f), Vec3f(1.0f,1.0f,1.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateVec3fParam("biofractal/julia", mBioFractalParams.mJulia.ptr(), Vec3f(-5.0f,-5.0f,-5.0f), Vec3f(5.0f,5.0f,5.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateVec3fParam("biofractal/lightdir", &mBioFractalParams.mLightDir, Vec3f(-1.0f,-1.0f,-1.0f), Vec3f(1.0f,1.0f,1.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "biofractal/scale", &mBioFractalParams.mScale )
                         .minValue(0.0f)
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "biofractal/angle", &mBioFractalParams.mRotAngle )
                         .minValue(0.0f)
                         .maxValue(90.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "biofractal/amplitude", &mBioFractalParams.mAmplitude )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "biofractal/detail", &mBioFractalParams.mDetail )
                         .minValue(0.0f)
                         .maxValue(0.1f));
    
    // fireball
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("fireball");
    mInterface->addParam(CreateBoolParam("fireball/audiodensity", &mFireballParams.mAudioRotation )
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("fireball/density", &mFireballParams.mDensity)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("fireball/rotationspeed", &mFireballParams.mRotationSpeed)
                         .oscReceiver(mName));
    
    // audio input
    mApp->getAudioInputHandler().setupInterface(mInterface, "global");
}

void ObjectShaders::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    
    for( FragShader* shader : mShaders )
    {
        if (shader)
        {
            shader->update(mTimeController.getDelta());
        }
    }
    
//    const float lows = mApp->getAudioInputHandler().getAverageVolumeLowFreq();
//    const float mids = mApp->getAudioInputHandler().getAverageVolumeMidFreq();
//    const float highs = mApp->getAudioInputHandler().getAverageVolumeHighFreq();
    
    // TODO: refactor
//    switch (mShaderType)
//    {
//        case SHADER_METAHEX:
//            mMetaHexParams.mLightTime += dt * mMetaHexParams.mLightSpeed;
//            mMetaHexParams.mObjTime += dt * mMetaHexParams.mSpeed;
//            if (mMetaHexParams.mAudioCoeffs)
//            {
//                mMetaHexParams.mCoeffecients.z = 0.5f + lows;
//                mMetaHexParams.mCoeffecients.y = 0.5f + mids*3.0f;
//                mMetaHexParams.mCoeffecients.x = 0.5f + highs*2.0f;
//            }
//            break;
//        
//        case SHADER_RETINA:
//            if (mRetinaParams.mAudioPattern)
//            {
//                mRetinaParams.mPatternAmp = 0.01f + 0.25f*mids;
//                mRetinaParams.mPatternFreq = 5.0f + 10.0f*highs;
//            }
//            break;
//        
//        case SHADER_BIOFRACTAL:
//            break;
//        
//        default:
//        break;
//    }
    
    // TODO: resolume can do this, maybe vdmx too, is it useful?
    if (mDrawOnSphere)
    {
        gl::pushMatrices();
        mShaderFbo.bindFramebuffer();
        gl::setMatricesWindow( mShaderFbo.getSize(), false );
        gl::setViewport( mShaderFbo.getBounds() );
        gl::enableDepthWrite();
        shaderPreDraw();
        Utils::drawTexturedRect( mApp->getViewportBounds() );
        shaderPostDraw();
        mShaderFbo.unbindFramebuffer();
        mShaderFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
        gl::popMatrices();
    }
}

void ObjectShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void ObjectShaders::shaderPreDraw()
{
    mColorMapTexture[mColorMapIndex].bind(0);
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().bindTexture(1);
    }
    mNoiseTexture.bind(2);
    
    gl::GlslProg shader = mShaders[mShaderType]->getShader();
    shader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
//    shader.uniform( "iColor1", mColor1);
//    shader.uniform( "iColor2", mColor2);
//    shader.uniform( "iColor3", mColor3);
//    shader.uniform( "iBackgroundAlpha", mBackgroundAlpha);
//    shader.uniform( "iTimeScale", mTimeController.getTimeScale() );
    
    mShaders[mShaderType]->setCustomParams( mApp->getAudioInputHandler() );
    
//    const float lows = mApp->getAudioInputHandler().getAverageVolumeLowFreq();
    
//    switch (mShaderType) {
//        case SHADER_METAHEX:
//            shader.uniform( "iObjTime", mMetaHexParams.mObjTime );
//            shader.uniform( "iLightTime", mMetaHexParams.mLightTime );
//            shader.uniform( "iNumObjects", mMetaHexParams.mNumObjects );
//            shader.uniform( "iRenderSteps", mMetaHexParams.mRenderSteps );
//            shader.uniform( "iQuality", mMetaHexParams.mQuality );
//            shader.uniform( "iCoefficients", mMetaHexParams.mCoeffecients );
//            break;
//            
//        case SHADER_BIOFRACTAL:
//            shader.uniform( "iIterations", mBioFractalParams.mIterations );
//            shader.uniform( "iJulia", mBioFractalParams.mJulia );
//            shader.uniform( "iRotation", mBioFractalParams.mRotation );
//            shader.uniform( "iScale", mBioFractalParams.mScale );
//            shader.uniform( "iRotAngle", mBioFractalParams.mRotAngle );
//            shader.uniform( "iAmplitude", mBioFractalParams.mAmplitude );
//            shader.uniform( "iLightDir", mBioFractalParams.mLightDir );
//            shader.uniform( "iDetail", mBioFractalParams.mDetail );
//            break;
//            
//        case SHADER_RETINA:
//        {
//            float dialation = mRetinaParams.mDialation * mRetinaParams.mDialationScale;
//            
//            if (mRetinaParams.mAudioDialation)
//            {
//                dialation = mRetinaParams.mDialation + lows * mRetinaParams.mDialationScale;
//            }
//        
//            shader.uniform( "iDialation", dialation );
//            shader.uniform( "iDialationScale", mRetinaParams.mDialationScale );
//            shader.uniform( "iPatternAmp", mRetinaParams.mPatternAmp );
//            shader.uniform( "iPatternFreq", mRetinaParams.mPatternFreq );
//            shader.uniform( "iScale", mRetinaParams.mScale );
//        }
//            break;
//            
//        case SHADER_FIREBALL:
//            shader.uniform( "iRotationSpeed", mFireballParams.mRotationSpeed );
//            shader.uniform( "iDensity", mFireballParams.mDensity );
//            shader.uniform( "iChannel0", 2 );
//            break;
//            
//        case SHADER_CLOUDS:
//            shader.uniform( "iChannel0", 2 );
//            break;
//            
//        default:
//            break;
//    }
}

void ObjectShaders::shaderPostDraw()
{
    gl::GlslProg shader = mShaders[mShaderType]->getShader();
    shader.unbind();
    
    mNoiseTexture.unbind();
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
    mColorMapTexture[mColorMapIndex].unbind();
}

void ObjectShaders::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    if (mDrawOnSphere)
    {
        gl::enableAlphaBlending();
        //gl::enable( GL_TEXTURE_2D );
        gl::enableDepthRead();
        gl::enableDepthWrite();
        gl::setMatrices( getCamera() );
        gl::setViewport( mApp->getViewportBounds() );
        gl::color( ColorA::white() );
        mShaderFbo.bindTexture();
        
        glEnable( GL_POLYGON_SMOOTH );
        //glEnable( GL_LIGHTING );
        //glEnable( GL_LIGHT0 );
        
        //GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        //glLightfv( GL_LIGHT0, GL_POSITION, light_position );
        
        gl::drawSphere(Vec3f::zero(), 640.0f, 64);
        //gl::drawSolidRect(mApp->getViewportBounds());
        
        mShaderFbo.unbindTexture();
        
        //glDisable( GL_LIGHT0 );
        //glDisable( GL_LIGHTING );
    }
    else
    {
        shaderPreDraw();
        Utils::drawTexturedRect( mApp->getViewportBounds() );
        shaderPostDraw();
    }
    
    gl::popMatrices();
}

void ObjectShaders::drawDebug()
{
    //mApp->getAudioInputHandler().drawDebug(mApp->getViewportSize());
    
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( mApp->getWindowSize() );
    
    const Vec2f size(128,72);
    Rectf preview( 100.0f, size.y - 200.0f, 180.0f, size.y - 120.0f );
    gl::draw( mShaderFbo.getTexture(), mShaderFbo.getBounds(), preview );
    
    gl::disable( GL_TEXTURE_2D );
}
