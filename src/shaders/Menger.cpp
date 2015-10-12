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
using namespace std;

#pragma mark - Scene

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
    
    mAmbientLight = 0.32184f;
    mDiffuseLight = 0.5f;
    mLight1Dir = Vec3f(1.0f, 1.0f, 0.858824f);
    mLight2Dir = Vec3f(1.0f, -1.0f, 1.0f);
    
    mAudioInputHandler.setup(false);
    
    mShaderIndex = 0;
    
    mShaders.push_back( new MengerShader() );
    mShaders.push_back( new PolychoraShader() );
    mShaders.push_back( new JuliaSpiralShader() );
    
    reset();
}

void Menger::reset()
{
    mTimeController.reset();
}

void Menger::setupInterface()
{
    vector<string> names;
    for( FragShader* shader : mShaders )
    {
        names.push_back(shader->getName());
    }
    mInterface->addEnum(CreateEnumParam( "shader", (int*)(&mShaderIndex) )
                        .maxValue(names.size())
                        .oscReceiver(getName())
                        .isVertical(), names);
    
    mTimeController.setupInterface(mInterface, getName());
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(getName()));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(getName()));
    
    mInterface->addParam(CreateVec3fParam("light1_dir", &mLight1Dir, Vec3f::one()*-1.0f, Vec3f::one()));
    mInterface->addParam(CreateVec3fParam("light2_dir", &mLight2Dir, Vec3f::one()*-1.0f, Vec3f::one()));
    
    mInterface->addParam(CreateFloatParam("ambient_light", &mAmbientLight)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam("diffuse_light", &mDiffuseLight)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    
    for( FragShader* shader : mShaders )
    {
        mInterface->gui()->addColumn();
        mInterface->gui()->addLabel( shader->getName() );
        shader->setupInterface(mInterface, getName());
    }
    
    mAudioInputHandler.setupInterface(mInterface, getName());
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
    gl::GlslProg shader = mShaders[mShaderIndex]->getShader();
    shader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    shader.uniform( "iColor1", Color(mColor1) );
    shader.uniform( "iColor2", Color(mColor2) );
    shader.uniform( "iAmbientLight", mAmbientLight );
    shader.uniform( "iDiffuseLight", mDiffuseLight );
    shader.uniform( "iLight1Dir", mLight1Dir );
    shader.uniform( "iLight2Dir", mLight2Dir );

    mShaders[mShaderIndex]->setCustomParams( mAudioInputHandler );
    
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
    mShaders[mShaderIndex]->getShader().unbind();
}

#pragma mark - Menger Shader

Menger::MengerShader::MengerShader()
: FragShader("menger", "menger_frag.glsl")
{
    mIterations = 7;
    mMaxSteps = 30;
    mFieldOfView = 1.0f;
    mScale = 3.0f;
    mJitter = 0.05f;
    mFudgeFactor = 0.7f;
    mPerspective = 2.0f;
    mMinDistance = 0.0009f;
    mNormalDistance = 0.0002f;
    mOffset = Vec3f(0.92858f,0.92858f,0.32858f);
    mOffsetXResponseBand = AudioInputHandler::BAND_NONE;
    mOffsetYResponseBand = AudioInputHandler::BAND_NONE;
    mOffsetZResponseBand = AudioInputHandler::BAND_NONE;
    mScaleResponseBand = AudioInputHandler::BAND_NONE;
}

void Menger::MengerShader::setupInterface(Interface *interface, const std::string &name)
{
    string oscName = name + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->addParam(CreateIntParam( "menger/iterations", &mIterations )
                        .minValue(1)
                        .maxValue(12)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "menger/max_steps", &mMaxSteps )
                        .minValue(10)
                        .maxValue(120)
                        .oscReceiver(oscName));
    interface->addParam(CreateVec3fParam("menger/offset", &mOffset, Vec3f::zero(), Vec3f::one()*2.0f)
                        .oscReceiver(oscName));
    
    interface->addEnum(CreateEnumParam( "menger/audio-offset.x", &mOffsetXResponseBand )
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(getName())
                       .sendFeedback(), bandNames);
    interface->addEnum(CreateEnumParam( "menger/audio-offset.y", &mOffsetYResponseBand )
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(getName())
                       .sendFeedback(), bandNames);
    interface->addEnum(CreateEnumParam( "menger/audio-offset.z", &mOffsetZResponseBand )
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(getName())
                       .sendFeedback(), bandNames);
    
    interface->addParam(CreateFloatParam("menger/scale", &mScale)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addEnum(CreateEnumParam( "menger/audio-scale", &mScaleResponseBand )
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(getName())
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("menger/fov", &mFieldOfView)
                        .minValue(0.4f)
                        .maxValue(20.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("menger/jitter", &mJitter)
                        .maxValue(0.4f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("menger/fudge", &mFudgeFactor)
                        .maxValue(3.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("menger/twist", &mPerspective)
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("menger/min_dist", &mMinDistance)
                        .maxValue(0.001f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("menger/norm_dist", &mNormalDistance)
                        .maxValue(0.001f)
                        .oscReceiver(oscName));
}

void Menger::MengerShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iMaxSteps", mMaxSteps );
    mShader.uniform( "iFieldOfView", mFieldOfView );
    mShader.uniform( "iJitter", mJitter );
    mShader.uniform( "iFudgeFactor", mFudgeFactor );
    mShader.uniform( "iPerspective", mPerspective );
    mShader.uniform( "iMinDistance", mMinDistance );
    mShader.uniform( "iNormalDistance", mNormalDistance );
    
    Vec3f offset = mOffset;
    
    offset.x *= 0.5f + 0.5f * audioInputHandler.getAverageVolumeByBand(mOffsetXResponseBand);
    offset.y *= 0.5f + 0.5f * audioInputHandler.getAverageVolumeByBand(mOffsetYResponseBand);
    offset.z *= 0.5f + 0.5f * audioInputHandler.getAverageVolumeByBand(mOffsetZResponseBand);
    
    mShader.uniform( "iOffset", offset );
    
    float scale = mScale * (0.5f + audioInputHandler.getAverageVolumeByBand(mScaleResponseBand));
    mShader.uniform( "iScale", scale );
}

#pragma mark - Polychora Shader

Menger::PolychoraShader::PolychoraShader()
: FragShader("polychora", "polychora_frag.glsl")
{
    mZone = 0;
    mMaxSteps = 30;
    mFieldOfView = 1.0f;
    mJitter = 0.05f;
    mFudgeFactor = 0.7f;
    mAngle = 0.5f;
    mMinDistance = 0.0009f;
    mNormalDistance = 0.0002f;
    mAngleResponseBand = AudioInputHandler::BAND_NONE;
}

void Menger::PolychoraShader::setupInterface(Interface *interface, const std::string &name)
{
    string oscName = name + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->addParam(CreateIntParam( "polychora/zone", &mZone )
                        .minValue(0)
                        .maxValue(5)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("polychora/angle", &mAngle)
                        .oscReceiver(oscName));
    interface->addEnum(CreateEnumParam( "polychora/audio-angle", &mAngleResponseBand )
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(getName())
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("polychora/fov", &mFieldOfView)
                        .minValue(0.4f)
                        .maxValue(20.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "polychora/max_steps", &mMaxSteps )
                        .minValue(2)
                        .maxValue(50)
                        .oscReceiver(oscName));
    
//    interface->addParam(CreateFloatParam("jitter", &mJitter)
//                        .maxValue(0.4f)
//                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("polychora/fudge", &mFudgeFactor)
                        .maxValue(3.0f)
                        .oscReceiver(oscName));
    
//    interface->addParam(CreateFloatParam("min_dist", &mMinDistance)
//                        .maxValue(0.001f)
//                        .oscReceiver(oscName));
//    interface->addParam(CreateFloatParam("norm_dist", &mNormalDistance)
//                        .maxValue(0.001f)
//                        .oscReceiver(oscName));
}

void Menger::PolychoraShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iZone", mZone );
    mShader.uniform( "iMaxSteps", mMaxSteps );
    mShader.uniform( "iFieldOfView", mFieldOfView );
    mShader.uniform( "iJitter", mJitter );
    mShader.uniform( "iFudgeFactor", mFudgeFactor );
    mShader.uniform( "iAngle", mAngle * audioInputHandler.getAverageVolumeByBand(mAngleResponseBand) );
    mShader.uniform( "iMinDistance", mMinDistance );
    mShader.uniform( "iNormalDistance", mNormalDistance );
}

#pragma mark - JuliaSpiralShader

Menger::JuliaSpiralShader::JuliaSpiralShader()
: FragShader("juliaspiral", "juliaspiral_frag.glsl")
{
    mScale = 1.0f;
    mFrequency = 0.01f;
    mOffset = Vec2f::zero();
//    mColor1 = ColorAf(0.6, 0.4, 0.0);
//    mColor2 = ColorAf(0.0, 0.6, 0.8);
    mColor3 = ColorAf(0.4f, 0.0f, 0.2f, 1.0f);
}

void Menger::JuliaSpiralShader::setupInterface(Interface *interface, const std::string &name)
{
    string oscName = name + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->addParam(CreateColorParam("juliaspiral/color3", &mColor3, kMinColor, kMaxColor).oscReceiver(getName()));
    interface->addParam(CreateVec2fParam("juliaspiral/offset", &mOffset, Vec2f::zero(), Vec2f::one()*3.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("juliaspiral/scale", &mScale)
                        .minValue(0.001f)
                        .maxValue(2.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("juliaspiral/frequency", &mFrequency)
                        .minValue(0.001f)
                        .maxValue(5.0f)
                        .oscReceiver(oscName));
//    interface->addEnum(CreateEnumParam( "audio-angle", &mAngleResponseBand )
//                       .maxValue(bandNames.size())
//                       .isVertical()
//                       .oscReceiver(getName())
//                       .sendFeedback(), bandNames);
    
}

void Menger::JuliaSpiralShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iColor3", Color(mColor3) );
    mShader.uniform( "iOffset", mOffset );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iFrequency", mFrequency );
}
