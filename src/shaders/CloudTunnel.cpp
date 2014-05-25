//
//  CloudTunnel.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-22.
//
//

#include "CloudTunnel.h"
#include "OculonApp.h"
#include "Utils.h"

using namespace ci;
using namespace std;

CloudTunnel::CloudTunnel()
: Scene("cloudtunnel")
{
}

CloudTunnel::~CloudTunnel()
{
}

void CloudTunnel::setup()
{
    Scene::setup();
    
    // params
    //mColor1 = ColorAf(0.07f, 0.1f, 0.15f, 1.0f);
    mColor1 = ColorAf(0.1f, 0.1f, 0.1f, 1.0f);
    mColor2 = ColorAf(50.f/256.f,100.f/256.f,255.f/256.f);
    
    mIterations = 80;
    mDensity = 0.35f;
    mFieldOfView = 2.0f;
    mNoiseLevel = 3.0f;
    mTunnelWidth = 1.5f;
    
    mDensityResponseBand = AudioInputHandler::BAND_NONE;
    mFOVResponseBand = AudioInputHandler::BAND_NONE;
    mNoiseLevelResponseBand = AudioInputHandler::BAND_NONE;
    mTunnelWidthResponseBand = AudioInputHandler::BAND_NONE;
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("cloudtunnel_frag.glsl");
    
    reset();
}

void CloudTunnel::reset()
{
    mTimeController.reset();
}

void CloudTunnel::setupInterface()
{
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(mName));
    
    mInterface->addParam(CreateIntParam("iterations", &mIterations)
                         .minValue(10)
                         .maxValue(200)
                         .oscReceiver(mName));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateFloatParam("density", &mDensity)
                         .minValue(0.02f)
                         .maxValue(2.0f)
                         .oscReceiver(mName));
    mInterface->addEnum(CreateEnumParam( "audio-density", &mDensityResponseBand )
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(mName)
                       .sendFeedback(), bandNames);
    
    mInterface->addParam(CreateFloatParam("fov", &mFieldOfView)
                         .minValue(1.0f)
                         .maxValue(15.0f)
                         .oscReceiver(mName));
    mInterface->addEnum(CreateEnumParam( "audio-fov", &mFOVResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), bandNames);
    
    mInterface->addParam(CreateFloatParam("noiselevel", &mNoiseLevel)
                         .minValue(2.0f)
                         .maxValue(10.0f)
                         .oscReceiver(mName));
    mInterface->addEnum(CreateEnumParam( "audio-noise", &mNoiseLevelResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), bandNames);
    
    mInterface->addParam(CreateFloatParam("tunnelwidth", &mTunnelWidth)
                         .minValue(0.5f)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    mInterface->addEnum(CreateEnumParam( "audio-width", &mTunnelWidthResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(mName)
                        .sendFeedback(), bandNames);
    
    mInterface->gui()->addColumn();
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void CloudTunnel::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
}

void CloudTunnel::draw()
{
    gl::pushMatrices();
    
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    Utils::drawTexturedRect( mApp->getViewportBounds() );
    
    shaderPostDraw();
    
    gl::popMatrices();
    
    gl::popMatrices();
}

void CloudTunnel::shaderPreDraw()
{
    mShader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    mShader.uniform( "iColor1", mColor1);
    mShader.uniform( "iColor2", mColor2);
    mShader.uniform( "iIterations", mIterations );
    
    mShader.uniform( "iFOV", mFieldOfView * mAudioInputHandler.getAverageVolumeByBand(mFOVResponseBand) );
    mShader.uniform( "iDensity", mDensity * mAudioInputHandler.getAverageVolumeByBand(mDensityResponseBand) );
    mShader.uniform( "iNoiseLevel", mNoiseLevel * mAudioInputHandler.getAverageVolumeByBand(mNoiseLevelResponseBand) );
    mShader.uniform( "iTunnelWidth", mTunnelWidth * mAudioInputHandler.getAverageVolumeByBand(mTunnelWidthResponseBand) );
}

void CloudTunnel::shaderPostDraw()
{
    mShader.unbind();
}