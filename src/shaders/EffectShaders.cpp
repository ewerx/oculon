//
//  EffectShaders.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-15.
//
//

#include "EffectShaders.h"

#include "OculonApp.h"
#include "Utils.h"

using namespace ci;
using namespace ci::app;
using namespace std;

EffectShaders::EffectShaders()
: Scene("effects")
, mCurrentEffect(0)
{
}

EffectShaders::~EffectShaders()
{
}

void EffectShaders::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(true);
    
    // effects
    mEffects.push_back( new CathodeRay() );
    mEffects.push_back( new Television() );
    mEffects.push_back( new VideoTape() );
    
    // inputs
    vector<Scene*> scenes;
    scenes.push_back( mApp->getScene("lines") );
    scenes.push_back( mApp->getScene("rings") );
    scenes.push_back( mApp->getScene("textureshaders") );
    scenes.push_back( mApp->getScene("audio") );
    scenes.push_back( mApp->getScene("objectshaders") );
    scenes.push_back( mApp->getScene("tilings") );
//    scenes.push_back( mApp->getScene("graviton") );
//    scenes.push_back( mApp->getScene("shadertest") );
    scenes.push_back( mApp->getScene("circlewave") );
    scenes.push_back( mApp->getScene("waves") );
    scenes.push_back( mApp->getScene("contours") );
    scenes.push_back( mApp->getScene("cells") );
    scenes.push_back( mApp->getScene("tilings") );
    
    for (Scene* scene : scenes )
    {
        if (scene)
        {
            mInputTextures.addTexture( scene->getName(), scene->getFboTexture() );
        }
    }
    
    // TODO: add syphon client as an input
    
    // dynamic noise
    mDynamicNoiseTexture.setup(256, 256);

    mNoiseTextures.addTexture( "static", "rgb_noise256.png" );
    mNoiseTextures.addTexture( "dynamic", mDynamicNoiseTexture.getTexture() );
    mNoiseTextures.addTexture( "audio", mAudioInputHandler.getTexture() );
    
    reset();
}

void EffectShaders::reset()
{
    mTimeController.reset();
}

void EffectShaders::setupInterface()
{
    mTimeController.setupInterface(mInterface, getName());
    
    // effects
    vector<string> effectNames;
    for( FragShader* effect : mEffects )
    {
        if (effect)
        {
            effectNames.push_back(effect->getName());
        }
    }
    mInterface->addEnum(CreateEnumParam( "effect", (int*)(&mCurrentEffect) )
                        .maxValue(effectNames.size())
                        .oscReceiver(getName())
                        .isVertical(), effectNames);
    
    // inputs
    mInputTextures.setupInterface( mInterface, getName(), "input" );
    
    // noise
    mNoiseTextures.setupInterface( mInterface, getName(), "noise" );
    
    mDynamicNoiseTexture.setupInterface(mInterface, getName());
    
    for( FragShader* effect : mEffects )
    {
        if (effect)
        {
            mInterface->gui()->addColumn();
            effect->setupInterface(mInterface, getName());
        }
    }
    
    mAudioInputHandler.setupInterface(mInterface, getName());
}

void EffectShaders::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    mDynamicNoiseTexture.update(dt);
    
    // update effect
}

void EffectShaders::draw()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    gl::pushMatrices();
    
    // pre-draw
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    mInputTextures.getTexture().bind(0);
    mNoiseTextures.getTexture().bind(1);
    
    gl::GlslProg shader = mEffects[mCurrentEffect]->getShader();
    shader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    shader.uniform( "inputTex", 0 );
    shader.uniform( "noiseTex", 1 );
    
    mEffects[mCurrentEffect]->setCustomParams( mAudioInputHandler );
    
    // draw
    Utils::drawTexturedRect( mApp->getViewportBounds() );

    // post-draw
    shader.unbind();
    
    mNoiseTextures.getTexture().unbind();
    mInputTextures.getTexture().unbind();
    
    gl::popMatrices();
    glPopAttrib();
}

#pragma mark - CRT

EffectShaders::CathodeRay::CathodeRay()
: FragShader("crt", "effect_crt.frag")
{
    mPowerBandThickness = 0.1; // percentage of v-size
    mPowerBandSpeed = -0.2;
    mPowerBandIntensity = 4.0;
    
    mPowerBandThicknessResponse = AudioInputHandler::BAND_NONE;
    mPowerBandIntensityResponse = AudioInputHandler::BAND_NONE;
    mPowerBandSpeedResponse = AudioInputHandler::BAND_NONE;
    mSignalNoiseResponse = AudioInputHandler::BAND_NONE;
    
    mSignalNoise = 0.8f;
    mScanlines = 120.0f;
    
    mTintColor = ColorAf(0.9f, 0.7f, 1.2f, 1.0f);
}

void EffectShaders::CathodeRay::setupInterface(Interface *interface, const std::string &prefix)
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->addParam(CreateFloatParam("powerband-thickness", &mPowerBandThickness)
                        .minValue(0.001f)
                        .oscReceiver(oscName));
    interface->addEnum(CreateEnumParam("thickness-audio", &mPowerBandThicknessResponse)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("powerband-speed", &mPowerBandSpeed)
                        .minValue(-1.0f)
                        .maxValue(1.0f)
                        .oscReceiver(oscName));
    interface->addEnum(CreateEnumParam("speed-audio", &mPowerBandSpeedResponse)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("powerband-intensity", &mPowerBandIntensity)
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addEnum(CreateEnumParam("intensity-audio", &mPowerBandIntensityResponse)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("signalnoise", &mSignalNoise)
                        .maxValue(2.0f)
                        .oscReceiver(oscName));
    interface->addEnum(CreateEnumParam("signal-audio", &mSignalNoiseResponse)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("scanlines", &mScanlines)
                        .minValue(4.0f)
                        .maxValue(300.f)
                        .oscReceiver(oscName));
    interface->addParam(CreateColorParam("tintcolor", &mTintColor, kMinColor, kMaxColor));
}

void EffectShaders::CathodeRay::setCustomParams(AudioInputHandler &audioInputHandler)
{
    mShader.uniform("iPowerBandThickness", mPowerBandThickness * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mPowerBandThicknessResponse)));
    mShader.uniform("iPowerBandIntensity", mPowerBandIntensity * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mPowerBandIntensityResponse)));
    mShader.uniform("iPowerBandSpeed", mPowerBandSpeed * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mPowerBandSpeedResponse)));
    mShader.uniform("iSignalNoise", mSignalNoise * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mSignalNoiseResponse)));
    mShader.uniform("iScanlines", mScanlines);
    mShader.uniform("iColor1", mTintColor);
}

#pragma mark - Television

EffectShaders::Television::Television()
: FragShader("television", "effect_tv.frag")
{
    mVerticalJerk       = 1.0f;
    mVerticalShift      = 1.0f;
    mBottomStatic       = 1.0f;
    mScanlines          = 1.0f;
    mColorShift         = 1.0f;
    mHorizontalFuzz     = 1.0f;
}

void EffectShaders::Television::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateFloatParam("VerticalJerk", &mVerticalJerk));
    interface->addParam(CreateFloatParam("VerticalShift", &mVerticalShift));
    interface->addParam(CreateFloatParam("BottomStatic", &mBottomStatic));
    interface->addParam(CreateFloatParam("Scanlines", &mScanlines));
    interface->addParam(CreateFloatParam("ColorShift", &mColorShift));
    interface->addParam(CreateFloatParam("HorizontalFuzz", &mHorizontalFuzz));
}

void EffectShaders::Television::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iVerticalJerk", mVerticalJerk);
    mShader.uniform("iVerticalShift", mVerticalShift);
    mShader.uniform("iBottomStatic", mBottomStatic);
    mShader.uniform("iScanlines", mScanlines);
    mShader.uniform("iColorShift", mColorShift);
    mShader.uniform("iHorizontalFuzz", mHorizontalFuzz);
}

#pragma mark - VCR

EffectShaders::VideoTape::VideoTape()
: FragShader("vcr", "effect_vcr.frag")
{
    
}

void EffectShaders::VideoTape::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void EffectShaders::VideoTape::setCustomParams( AudioInputHandler& audioInputHandler )
{
}
