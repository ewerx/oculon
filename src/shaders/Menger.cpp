//
//  Menger.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-17.
//
//

#include "Menger.h"
#include "OculonApp.h"
#include "Utils.h"

using namespace ci;

Menger::Menger()
: Scene("menger")
{
}

Menger::~Menger()
{
}

void Menger::setup()
{
    Scene::setup();
    
    // params
    mColor1 = ColorAf(1.0f, 1.0f, 0.858824f);
    mColor2 = ColorAf(0.0f, 0.333333f, 1.0f);
    
    mIterations = 7;
    mMaxSteps = 30;
    mFieldOfView = 1.0f;
    mScale = 3.0f;
    mJitter = 0.05f;
    mFudgeFactor = 0.7f;
    mPerspective = 2.0f;
    mMinDistance = 0.0009f;
    mNormalDistance = 0.0002f;
    
    mAmbientLight = 0.32184f;
    mDiffuseLight = 0.5f;
    mLight1Dir = Vec3f(1.0f, 1.0f, 0.858824f);
    mLight2Dir = Vec3f(1.0f, -1.0f, 1.0f);
    mOffset = Vec3f(0.92858f,0.92858f,0.32858f);
    
    mAudioInputHandler.setup(true);
    
    mShader = loadFragShader("menger_frag.glsl");
    
    reset();
}

void Menger::reset()
{
    mTimeController.reset();
}

void Menger::setupInterface()
{
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(mName));
    
    mInterface->addParam(CreateVec3fParam("light1_dir", &mLight1Dir, Vec3f::one()*-1.0f, Vec3f::one()));
    mInterface->addParam(CreateVec3fParam("light2_dir", &mLight2Dir, Vec3f::one()*-1.0f, Vec3f::one()));
    mInterface->addParam(CreateVec3fParam("offset", &mOffset, Vec3f::zero(), Vec3f::one()*2.0f));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateIntParam( "iterations", &mIterations )
                         .minValue(1)
                         .maxValue(12));
    mInterface->addParam(CreateIntParam( "max_steps", &mMaxSteps )
                         .minValue(10)
                         .maxValue(120));
    mInterface->addParam(CreateFloatParam("mScale", &mScale)
                         .minValue(1.0f)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("fov", &mFieldOfView)
                         .minValue(0.4f)
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("jitter", &mJitter)
                         .maxValue(0.4f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("fudge", &mFudgeFactor)
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("twist", &mPerspective)
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("min_dist", &mMinDistance)
                         .maxValue(0.001f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("norm_dist", &mNormalDistance)
                         .maxValue(0.001f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("ambient_light", &mAmbientLight)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("diffuse_light", &mDiffuseLight)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Menger::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
}

void Menger::draw()
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

void Menger::shaderPreDraw()
{
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    mShader.uniform( "iColor1", Color(mColor1) );
    mShader.uniform( "iColor2", Color(mColor2) );
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iMaxSteps", mMaxSteps );
    mShader.uniform( "iFieldOfView", mFieldOfView );
    mShader.uniform( "iJitter", mJitter );
    mShader.uniform( "iFudgeFactor", mFudgeFactor );
    mShader.uniform( "iPerspective", mPerspective );
    mShader.uniform( "iMinDistance", mMinDistance );
    mShader.uniform( "iNormalDistance", mNormalDistance );
    mShader.uniform( "iAmbientLight", mAmbientLight );
    mShader.uniform( "iDiffuseLight", mDiffuseLight );
    mShader.uniform( "iLight1Dir", mLight1Dir );
    mShader.uniform( "iLight2Dir", mLight2Dir );
    mShader.uniform( "iOffset", mOffset );
    
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

void Menger::shaderPostDraw()
{
    mShader.unbind();
}