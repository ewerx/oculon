//
//  RootFract.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-13.
//
//

#include "RootFract.h"
#include "OculonApp.h"
#include "Utils.h"

using namespace ci;
using namespace std;

RootFract::RootFract()
: Scene("rootfract")
{
}

RootFract::~RootFract()
{
}

void RootFract::setup()
{
    Scene::setup();
    
    // params
    mColor1 = ColorAf(0.0f,0.3f,1.0f);
    mColor2 = ColorAf(0.0f,0.5f,1.0f);
    mColor3 = ColorAf(1.0f,0.2f,0.0f);
    
    mIterations         = 27;
    mScale              = -0.4f;
    mZoom               = 5.0f;
    mJulia              = Vec2f(2.2f, 0.75f);
    mOrbitTraps         = Vec3f(0.8f, 0.5f, -0.01f);//mOrbitTraps         = Vec3f(.155f, .144f, 0.015f);
    mFrequency          = Vec3f(5.f,8.f,20.f);
    mAmplitude          = Vec3f(.03f,.03f,.01f);
    mSpeed              = Vec3f(20.f,20.f,40.f);
    
    mSaturation         = 0.35f;
    mBrightness         = 0.9f;
    mContrast           = 1.35f;
    mMinBrightness      = 0.3f;
    mTrapWidths         = Vec3f(.2f, .2f, .3f);
    mTrapBrightness     = Vec3f(1.0f, 0.8f, 0.7f);//mTrapBrightness     = Vec3f(2.2f, 1.7f, 1.0f);
    mTrapContrast       = Vec3f(5.0f, 10.0f, 5.0f);//Vec3f(0.25f, 2.0f, 5.0f);
    
    mFreqResponseBand   = AudioInputHandler::BAND_NONE;
    mAmpResponseBand    = AudioInputHandler::BAND_NONE;
    
    mAudioInputHandler.setup(false);
    
    //mShader = loadFragShader("rootfract_frag.glsl");
    mShader = loadFragShader("fractraps1_frag.glsl");
    
    reset();
}

void RootFract::reset()
{
    mTimeController.reset();
}

void RootFract::setupInterface()
{
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    mTimeController.setupInterface(mInterface, getName());
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(getName()));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(getName()));
    mInterface->addParam(CreateColorParam("color2", &mColor3, kMinColor, kMaxColor).oscReceiver(getName()));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateIntParam( "iterations", &mIterations )
                         .maxValue(120));
    mInterface->addParam(CreateFloatParam("scale", &mScale)
                         .minValue(-0.6f)
                         .maxValue(-0.2f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("zoom", &mZoom)
                         .minValue(1.0f)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    
    //mInterface->addParam(CreateVec2fParam("julia", &mJulia, Vec2f(0.1f,0.1f), Vec2f(10.0f,10.0f)));
    mInterface->addParam(CreateFloatParam("julia-x", &mJulia.x)
                         .minValue(0.1f)
                         .maxValue(10.0f));
    mInterface->addParam(CreateFloatParam("julia-y", &mJulia.y)
                         .minValue(0.1f)
                         .maxValue(10.0f));
    mInterface->addParam(CreateVec3fParam("traps", &mOrbitTraps, Vec3f::zero(), Vec3f::one()));
    mInterface->addParam(CreateVec3fParam("freq", &mFrequency, Vec3f::zero(), Vec3f::one()*20.0f));
    mInterface->addEnum(CreateEnumParam( "audio-freq", &mFreqResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    
    mInterface->addParam(CreateVec3fParam("amp", &mAmplitude, Vec3f::zero(), Vec3f::one()));
    mInterface->addEnum(CreateEnumParam( "audio-amp", &mAmpResponseBand )
                        .maxValue(bandNames.size())
                        .isVertical()
                        .oscReceiver(getName())
                        .sendFeedback(), bandNames);
    mInterface->addParam(CreateVec3fParam("speed", &mSpeed, Vec3f::zero(), Vec3f::one()*40.0f));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateFloatParam("saturation", &mSaturation)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("brightness", &mBrightness)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("contrast", &mContrast)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("min_brightness", &mMinBrightness)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateVec3fParam("trap_width", &mTrapWidths, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("trap_bright", &mTrapBrightness, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("trap_contrast", &mTrapContrast, Vec3f::zero(), Vec3f(13.0f,13.0f,13.0f)));
    
    mAudioInputHandler.setupInterface(mInterface, getName());
}

void RootFract::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
}

void RootFract::draw()
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

void RootFract::shaderPreDraw()
{
    mShader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    Vec3f freq = mFrequency;
    Vec3f amp = mAmplitude;
    
    if (mFreqResponseBand != AudioInputHandler::BAND_NONE)
    {
        int band = mFreqResponseBand;
        freq.x = freq.x * (0.5f + 0.5f * mAudioInputHandler.getAverageVolumeByBand(band));
        band = (band + 1) % AudioInputHandler::BAND_COUNT;
        freq.y = freq.y * (0.5f + 0.5f * mAudioInputHandler.getAverageVolumeByBand(band));
        band = (band + 1) % AudioInputHandler::BAND_COUNT;
        freq.z = freq.z * (0.5f + 0.5f * mAudioInputHandler.getAverageVolumeByBand(band));
    }
    
    if (mAmpResponseBand != AudioInputHandler::BAND_NONE)
    {
        int band = mAmpResponseBand;
        amp.x = amp.x * (0.5f + 0.5f * mAudioInputHandler.getAverageVolumeByBand(band));
        band = (band + 1) % AudioInputHandler::BAND_COUNT;
        amp.y = amp.y * (0.5f + 0.5f * mAudioInputHandler.getAverageVolumeByBand(band));
        band = (band + 1) % AudioInputHandler::BAND_COUNT;
        amp.z = amp.z * (0.5f + 0.5f * mAudioInputHandler.getAverageVolumeByBand(band));
    }
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    mShader.uniform( "iColor1", Colorf(mColor1));
    mShader.uniform( "iColor2", Colorf(mColor2));
    mShader.uniform( "iColor3", Colorf(mColor3));
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iZoom", mZoom );
    mShader.uniform( "iJulia", mJulia );
    mShader.uniform( "iOrbitTraps", mOrbitTraps );
    mShader.uniform( "iFrequency", freq );
    mShader.uniform( "iAmplitude", amp );
    mShader.uniform( "iSpeed", mSpeed );
    
    mShader.uniform( "iSaturation", mSaturation );
    mShader.uniform( "iBrightness", mBrightness );
    mShader.uniform( "iContrast", mContrast );
    mShader.uniform( "iMinBrightness", mMinBrightness );
    mShader.uniform( "iTrapWidths", mTrapWidths );
    mShader.uniform( "iTrapBrightness", mTrapBrightness );
    mShader.uniform( "iTrapContrast", mTrapContrast );
    
//    float distortion = mDistortion;
//    if (mAudioDistortion)
//    {
//        float audioLevel = 0.0f;
//        switch (mResponseBand) {
//            case 0:
//            audioLevel = mAudioInputHandler.getAverageVolumeLowFreq();
//            break;
//            case 1:
//            audioLevel = mAudioInputHandler.getAverageVolumeMidFreq();
//            break;
//            case 2:
//            audioLevel = mAudioInputHandler.getAverageVolumeHighFreq();
//            break;
//            default:
//            break;
//        }
//        distortion += 10.0f * audioLevel;
//        distortion = math<float>::min(distortion, 1.0f);
//    }
//    mShader.uniform( "distortion", distortion );
}

void RootFract::shaderPostDraw()
{
    mShader.unbind();
}
