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
, mCurrentInputTexture(0)
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
    mEffects.push_back( new TelevisionEffect() );
    
    // inputs
    vector<Scene*> scenes;
    scenes.push_back( mApp->getScene("lines") );
    scenes.push_back( mApp->getScene("rings") );
    scenes.push_back( mApp->getScene("textureshaders") );
    scenes.push_back( mApp->getScene("audio") );
    scenes.push_back( mApp->getScene("objectshaders") );
    scenes.push_back( mApp->getScene("tilings") );
    scenes.push_back( mApp->getScene("graviton") );
    scenes.push_back( mApp->getScene("shadertest") );
    scenes.push_back( mApp->getScene("circlewave") );
    
    for (Scene* scene : scenes )
    {
        if (scene)
        {
            mInputTextures.push_back( make_pair( scene->getName(), scene->getFboTexture() ) );
        }
    }
    
    // TODO: add syphon client as an input
    
    // dynamic noise
    mNoiseTexture.setup(256, 256);
    
    gl::Texture::Format format;
	format.setWrap( GL_REPEAT, GL_REPEAT );

    mCurrentNoiseTex = 0;
    mNoiseTextures.push_back( make_pair("static", gl::Texture( loadImage( loadResource( "rgb_noise256.png" ) ), format ) ));
    mNoiseTextures.push_back( make_pair("dynamic", mNoiseTexture.getTexture() ));
    mNoiseTextures.push_back( make_pair("audio", mAudioInputHandler.getTexture() ));
    
    reset();
}

void EffectShaders::reset()
{
    mTimeController.reset();
}

void EffectShaders::setupInterface()
{
    mTimeController.setupInterface(mInterface, mName);
    
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
    vector<string> inputNames;
    for( tNamedTexture namedTex : mInputTextures )
    {
        inputNames.push_back(namedTex.first);
    }
    mInterface->addEnum(CreateEnumParam( "input", (int*)(&mCurrentInputTexture) )
                        .maxValue(inputNames.size())
                        .oscReceiver(getName())
                        .isVertical(), inputNames);
    
    // inputs
    vector<string> noiseTexNames;
    for( tNamedTexture namedTex : mNoiseTextures )
    {
        noiseTexNames.push_back(namedTex.first);
    }
    mInterface->addEnum(CreateEnumParam( "noise", (int*)(&mCurrentNoiseTex) )
                        .maxValue(noiseTexNames.size())
                        .oscReceiver(getName())
                        .isVertical(), noiseTexNames);
    
    mNoiseTexture.setupInterface(mInterface, mName);
    
    for( FragShader* effect : mEffects )
    {
        if (effect)
        {
            mInterface->gui()->addColumn();
            effect->setupInterface(mInterface, mName);
        }
    }
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void EffectShaders::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    mNoiseTexture.update(dt);
    
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
    
    mInputTextures[mCurrentInputTexture].second.bind(0);
    mNoiseTextures[mCurrentNoiseTex].second.bind(1);
    
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
    
    mNoiseTextures[mCurrentNoiseTex].second.unbind();
    mInputTextures[mCurrentInputTexture].second.unbind();
    
    gl::popMatrices();
    glPopAttrib();
}

#pragma mark  - Effects

TelevisionEffect::TelevisionEffect()
: FragShader("television", "crtinterference_frag.glsl")
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

void TelevisionEffect::setupInterface(Interface *interface, const std::string &prefix)
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

void TelevisionEffect::setCustomParams(AudioInputHandler &audioInputHandler)
{
    
    
    mShader.uniform("iPowerBandThickness", mPowerBandThickness * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mPowerBandThicknessResponse)));
    mShader.uniform("iPowerBandIntensity", mPowerBandIntensity * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mPowerBandIntensityResponse)));
    mShader.uniform("iPowerBandSpeed", mPowerBandSpeed * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mPowerBandSpeedResponse)));
    mShader.uniform("iSignalNoise", mSignalNoise * (0.5f + 0.5f*audioInputHandler.getAverageVolumeByBand(mSignalNoiseResponse)));
    mShader.uniform("iScanlines", mScanlines);
    mShader.uniform("iColor1", mTintColor);
}
