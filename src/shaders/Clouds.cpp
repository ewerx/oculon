//
//  Clouds.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-22.
//
//

#include "Clouds.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Utils.h"

using namespace ci;
using namespace std;

Clouds::Clouds()
: TextureShaders("clouds")
{
    
}

Clouds::~Clouds()
{
}

void Clouds::setupShaders()
{
    //mColor1 = ColorAf(0.07f, 0.1f, 0.15f, 1.0f);
    mColor1 = ColorAf(0.1f, 0.1f, 0.1f, 1.0f);
    mColor2 = ColorAf(50.f/256.f,100.f/256.f,255.f/256.f);
    
    mShaderType = 0;
    
//    mShaders.push_back( new CloudTunnel() );
    mShaders.push_back( new PlasmaFog() );
    
    // noise
    mDynamicNoiseTexture.setup(256, 256);
    
    mNoiseTextures.addTexture( "static", "rgb_noise256.png" );
    mNoiseTextures.addTexture( "dynamic", mDynamicNoiseTexture.getTexture() );
    mNoiseTextures.addTexture( "audio", mAudioInputHandler.getTexture() );
}

void Clouds::setupInterface()
{
    mNoiseTextures.setupInterface(mInterface, getName(), "noise");
    mDynamicNoiseTexture.setupInterface(mInterface, getName());
    
    TextureShaders::setupInterface();
}

void Clouds::update(double dt)
{
    setInputTexture(make_shared<gl::Texture>(mNoiseTextures.getTexture()));
    
    TextureShaders::update(dt);
}

#pragma mark - CloudTunnel

Clouds::CloudTunnel::CloudTunnel()
: FragShader("CloudTunnel", "cloudtunnel_frag.glsl")
{
    mIterations = 80;
    mDensity = 0.35f;
    mFieldOfView = 2.0f;
    mNoiseLevel = 3.0f;
    mTunnelWidth = 1.5f;
    
    mDensityResponseBand = AudioInputHandler::BAND_NONE;
    mFOVResponseBand = AudioInputHandler::BAND_NONE;
    mNoiseLevelResponseBand = AudioInputHandler::BAND_NONE;
    mTunnelWidthResponseBand = AudioInputHandler::BAND_NONE;
}

void Clouds::CloudTunnel::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam("iterations", &mIterations)
                         .minValue(10)
                         .maxValue(200)
                         .oscReceiver(getName()));
    
    interface->gui()->addColumn();
    interface->addParam(CreateFloatParam("density", &mDensity)
                         .minValue(0.02f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam( "audio-density", &mDensityResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    interface->addParam(CreateFloatParam("fov", &mFieldOfView)
                         .minValue(1.0f)
                         .maxValue(15.0f)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam( "audio-fov", &mFOVResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    interface->addParam(CreateFloatParam("noiselevel", &mNoiseLevel)
                         .minValue(2.0f)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam( "audio-noise", &mNoiseLevelResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    interface->addParam(CreateFloatParam("tunnelwidth", &mTunnelWidth)
                         .minValue(0.5f)
                         .maxValue(4.0f)
                         .oscReceiver(getName()));
    interface->addEnum(CreateEnumParam( "audio-width", &mTunnelWidthResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
}

void Clouds::CloudTunnel::update(double dt)
{
    
}

void Clouds::CloudTunnel::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iIterations", mIterations );
    
    mShader.uniform( "iFOV", mFieldOfView * audioInputHandler.getAverageVolumeByBand(mFOVResponseBand) );
    mShader.uniform( "iDensity", mDensity * audioInputHandler.getAverageVolumeByBand(mDensityResponseBand) );
    mShader.uniform( "iNoiseLevel", mNoiseLevel * audioInputHandler.getAverageVolumeByBand(mNoiseLevelResponseBand) );
    mShader.uniform( "iTunnelWidth", mTunnelWidth * audioInputHandler.getAverageVolumeByBand(mTunnelWidthResponseBand) );
}

#pragma mark - MultiWave

Clouds::PlasmaFog::PlasmaFog()
: FragShader("plasmafog", "plasmafog.frag")
{
    
}

void Clouds::PlasmaFog::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
}

void Clouds::PlasmaFog::update(double dt)
{
    
}

void Clouds::PlasmaFog::setCustomParams( AudioInputHandler& audioInputHandler )
{
}
