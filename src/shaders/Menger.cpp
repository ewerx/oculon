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
}

void Menger::MengerShader::setupInterface(Interface *interface, const std::string &name)
{
    string oscName = name + "/" + mName;
    
    interface->addParam(CreateIntParam( "iterations", &mIterations )
                        .minValue(1)
                        .maxValue(12)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "max_steps", &mMaxSteps )
                        .minValue(10)
                        .maxValue(120)
                        .oscReceiver(oscName));
    interface->addParam(CreateVec3fParam("offset", &mOffset, Vec3f::zero(), Vec3f::one()*2.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("mScale", &mScale)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("fov", &mFieldOfView)
                        .minValue(0.4f)
                        .maxValue(20.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("jitter", &mJitter)
                        .maxValue(0.4f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("fudge", &mFudgeFactor)
                        .maxValue(3.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("twist", &mPerspective)
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("min_dist", &mMinDistance)
                        .maxValue(0.001f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("norm_dist", &mNormalDistance)
                        .maxValue(0.001f)
                        .oscReceiver(oscName));
}

void Menger::MengerShader::setCustomParams()
{
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iMaxSteps", mMaxSteps );
    mShader.uniform( "iFieldOfView", mFieldOfView );
    mShader.uniform( "iJitter", mJitter );
    mShader.uniform( "iFudgeFactor", mFudgeFactor );
    mShader.uniform( "iPerspective", mPerspective );
    mShader.uniform( "iMinDistance", mMinDistance );
    mShader.uniform( "iNormalDistance", mNormalDistance );
    mShader.uniform( "iOffset", mOffset );
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
    mAntiAlias = 0.5f;
    mMinDistance = 0.0009f;
    mNormalDistance = 0.0002f;
}

void Menger::PolychoraShader::setupInterface(Interface *interface, const std::string &name)
{
    string oscName = name + "/" + mName;
    
    interface->addParam(CreateIntParam( "zone", &mZone )
                        .minValue(0)
                        .maxValue(5)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "max_steps", &mMaxSteps )
                        .minValue(2)
                        .maxValue(50)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("fov", &mFieldOfView)
                        .minValue(0.4f)
                        .maxValue(20.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("jitter", &mJitter)
                        .maxValue(0.4f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("fudge", &mFudgeFactor)
                        .maxValue(3.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("anti-alias", &mAntiAlias)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("min_dist", &mMinDistance)
                        .maxValue(0.001f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam("norm_dist", &mNormalDistance)
                        .maxValue(0.001f)
                        .oscReceiver(oscName));
}

void Menger::PolychoraShader::setCustomParams()
{
    mShader.uniform( "iZone", mZone );
    mShader.uniform( "iMaxSteps", mMaxSteps );
    mShader.uniform( "iFieldOfView", mFieldOfView );
    mShader.uniform( "iJitter", mJitter );
    mShader.uniform( "iFudgeFactor", mFudgeFactor );
    mShader.uniform( "iAntiAlias", mAntiAlias );
    mShader.uniform( "iMinDistance", mMinDistance );
    mShader.uniform( "iNormalDistance", mNormalDistance );
}

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
    
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(mName));
    
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
        shader->setupInterface(mInterface, mName);
    }
    
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
    gl::GlslProg shader = mShaders[mShaderIndex]->getShader();
    shader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    shader.uniform( "iColor1", Color(mColor1) );
    shader.uniform( "iColor2", Color(mColor2) );
    shader.uniform( "iAmbientLight", mAmbientLight );
    shader.uniform( "iDiffuseLight", mDiffuseLight );
    shader.uniform( "iLight1Dir", mLight1Dir );
    shader.uniform( "iLight2Dir", mLight2Dir );

    mShaders[mShaderIndex]->setCustomParams();
    
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
