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
//    mShaderFbo = gl::Fbo( mApp->getViewportWidth(), mApp->getViewportHeight(), format );
    
    setupShaders();
    
    // color maps
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex1.jpg" ) ) ) ) );
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex2.jpg" ) ) ) ) );
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex3.jpg" ) ) ) ) );
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex4.jpg" ) ) ) ) );
    mColorMapIndex = 0;
    
    mNoiseTexture = gl::Texture( loadImage( loadResource( "gaussian_noise_256_3c.png" ) ) );

//    mDrawOnSphere = false;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mColor3 = ColorA::white();
    
    mBackgroundAlpha = 0.0f;
    
    reset();
}

void ObjectShaders::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new MetaballsShader() );
    mShaders.push_back( new LissajousShader() );
    mShaders.push_back( new GyroidShader() );
    mShaders.push_back( new BioFractalShader() );
    mShaders.push_back( new RetinaShader() );
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
    
    vector<string> colorMapNames;
    for( tNamedTexture namedTex : mColorMaps )
    {
        colorMapNames.push_back(namedTex.first);
    }
    mInterface->addEnum(CreateEnumParam( "colormap", (int*)(&mColorMapIndex) )
                        .maxValue(colorMapNames.size())
                        .oscReceiver(mName)
                        .isVertical(), colorMapNames);
    
    // custom params
    for( FragShader* shader : mShaders )
    {
        if (shader)
        {
            mInterface->gui()->addColumn();
            shader->setupInterface(mInterface, mName);
        }
    }
    
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
    
    // TODO: resolume can do this, maybe vdmx too, is it useful?
//    if (mDrawOnSphere)
//    {
//        gl::pushMatrices();
//        mShaderFbo.bindFramebuffer();
//        gl::setMatricesWindow( mShaderFbo.getSize(), false );
//        gl::setViewport( mShaderFbo.getBounds() );
//        gl::enableDepthWrite();
//        shaderPreDraw();
//        Utils::drawTexturedRect( mApp->getViewportBounds() );
//        shaderPostDraw();
//        mShaderFbo.unbindFramebuffer();
//        mShaderFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
//        gl::popMatrices();
//    }
}

void ObjectShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void ObjectShaders::shaderPreDraw()
{
    mColorMaps[mColorMapIndex].second.bind(0);
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
    shader.uniform( "iColor1", mColor1);
    shader.uniform( "iColor2", mColor2);
    shader.uniform( "iColor3", mColor3);
    shader.uniform( "iBackgroundAlpha", mBackgroundAlpha);
    shader.uniform( "iTimeScale", mTimeController.getTimeScale() );
    
    mShaders[mShaderType]->setCustomParams( mApp->getAudioInputHandler() );
    
//    const float lows = mApp->getAudioInputHandler().getAverageVolumeLowFreq();
    
//    switch (mShaderType) {
//            
//        case SHADER_BIOFRACTAL:
//
//            break;
//            
//        case SHADER_RETINA:
//        {
//            float dialation = mDialation * mDialationScale;
//            
//            if (mAudioDialation)
//            {
//                dialation = mDialation + lows * mDialationScale;
//            }
//        
//            mShader.uniform( "iDialation", dialation );
//            mShader.uniform( "iDialationScale", mDialationScale );
//            mShader.uniform( "iPatternAmp", mPatternAmp );
//            mShader.uniform( "iPatternFreq", mPatternFreq );
//            mShader.uniform( "iScale", mScale );
//        }
//            break;
//            
//        case SHADER_FIREBALL:
//            mShader.uniform( "iRotationSpeed", mFireballParams.mRotationSpeed );
//            mShader.uniform( "iDensity", mFireballParams.mDensity );
//            mShader.uniform( "iChannel0", 2 );
//            break;
//            
//        case SHADER_CLOUDS:
//            mShader.uniform( "iChannel0", 2 );
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
    mColorMaps[mColorMapIndex].second.unbind();
}

void ObjectShaders::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
//    if (mDrawOnSphere)
//    {
//        gl::enableAlphaBlending();
//        //gl::enable( GL_TEXTURE_2D );
//        gl::enableDepthRead();
//        gl::enableDepthWrite();
//        gl::setMatrices( getCamera() );
//        gl::setViewport( mApp->getViewportBounds() );
//        gl::color( ColorA::white() );
//        mShaderFbo.bindTexture();
//        
//        glEnable( GL_POLYGON_SMOOTH );
//        //glEnable( GL_LIGHTING );
//        //glEnable( GL_LIGHT0 );
//        
//        //GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//        //glLightfv( GL_LIGHT0, GL_POSITION, light_position );
//        
//        gl::drawSphere(Vec3f::zero(), 640.0f, 64);
//        //gl::drawSolidRect(mApp->getViewportBounds());
//        
//        mShaderFbo.unbindTexture();
//        
//        //glDisable( GL_LIGHT0 );
//        //glDisable( GL_LIGHTING );
//    }
//    else
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
    
//    gl::enable( GL_TEXTURE_2D );
//    gl::setMatricesWindow( mApp->getWindowSize() );
//    
//    const Vec2f size(128,72);
//    Rectf preview( 100.0f, size.y - 200.0f, 180.0f, size.y - 120.0f );
//    gl::draw( mShaderFbo.getTexture(), mShaderFbo.getBounds(), preview );
//    
//    gl::disable( GL_TEXTURE_2D );
}

#pragma mark - Hexballs

MetaballsShader::MetaballsShader()
: FragShader("metaballs", "metahex_frag.glsl")
, mQuality(4.0f)
, mRenderSteps(33)
, mLightSpeed(1.0f)
, mSpeed(1.0f)
, mNumObjects(9)
, mCoeffecients(Vec3f(0.967f,0.423f,0.76321f))
, mResponseBand(AudioInputHandler::BAND_NONE)
, mObjTime(0.0f)
, mLightTime(0.0f)
{
    
}

void MetaballsShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(mName);
    interface->addParam(CreateIntParam( "metahex/objects", &mNumObjects )
                        .maxValue(24)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "metahex/steps", &mRenderSteps )
                        .maxValue(128)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "metahex/quality", &mQuality )
                        .maxValue(16)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "metahex/speed", &mSpeed )
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "metahex/lightspeed", &mLightSpeed )
                        .maxValue(10.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateVec3fParam("metahex/coeffs", &mCoeffecients, Vec3f::zero(), Vec3f(1.0f,1.0f,1.0f))
                        .oscReceiver(mName));
    interface->addEnum(CreateEnumParam("response-band", &mResponseBand)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
}

void MetaballsShader::update(double dt)
{
    mLightTime += dt * mLightSpeed;
    mObjTime += dt * mSpeed;
}

void MetaballsShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    Vec3f coeffs = mCoeffecients;
    coeffs *= audioInputHandler.getAverageVolumeByBand(mResponseBand);
    
    mShader.uniform( "iObjTime", mObjTime );
    mShader.uniform( "iLightTime", mLightTime );
    mShader.uniform( "iNumObjects", mNumObjects );
    mShader.uniform( "iRenderSteps", mRenderSteps );
    mShader.uniform( "iQuality", mQuality );
    mShader.uniform( "iCoefficients", coeffs );
}

#pragma mark - Lissajous

LissajousShader::LissajousShader()
: FragShader("lissajous", "lissajous_frag.glsl")
, mFrequencyX(2.0f)
, mFrequencyY(3.0f)
, mFrequencyXShift(0.0f)
, mFrequencyYShift(0.0f)
, mScale(0.8f)
, mResponseBandX(AudioInputHandler::BAND_NONE)
, mResponseBandY(AudioInputHandler::BAND_NONE)
, mColor(ColorAf(0.2f, 1.0f, 0.1f, 1.0f))
{
    
}

void LissajousShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(mName);
    interface->addParam(CreateIntParam( "frequency-x", &mFrequencyX )
                        .minValue(1)
                        .maxValue(32)
                        .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "frequency-y", &mFrequencyY )
                        .minValue(1)
                        .maxValue(32)
                        .oscReceiver(oscName));
    
    interface->addParam(CreateFloatParam( "frequency-x-shift", &mFrequencyXShift )
                        .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "frequency-y-shift", &mFrequencyYShift )
                        .oscReceiver(oscName));
    
    interface->addEnum(CreateEnumParam("x-band", &mResponseBandX)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addEnum(CreateEnumParam("y-band", &mResponseBandY)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam("scale", &mScale)
                        .minValue(0.05f)
                        .maxValue(2.0f)
                        .oscReceiver(oscName));
    interface->addParam(CreateColorParam("color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(oscName));
}

void LissajousShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform("iScale", mScale);
    
    mShader.uniform("iColor", mColor);
    
    float frequencyX = (int)(mFrequencyX * audioInputHandler.getAverageVolumeByBand(mResponseBandX)) + mFrequencyXShift;
    float frequencyY = (int)(mFrequencyY * audioInputHandler.getAverageVolumeByBand(mResponseBandY)) + mFrequencyYShift;
    
    mShader.uniform("iFrequencyX", frequencyX);
    mShader.uniform("iFrequencyY", frequencyY);
}

#pragma mark - Retina

RetinaShader::RetinaShader()
: FragShader("retina", "retina_frag.glsl")
{
    mDialation = 0.2f;
    mDialationScale = 1.0f;
    mPatternAmp = 0.05f;
    mPatternFreq = 20.0f;
    mDialationBand = AudioInputHandler::BAND_NONE;
    mPatternBand = AudioInputHandler::BAND_NONE;
    mScale = 1.0f;
}

void RetinaShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(mName);
    interface->addParam(CreateFloatParam("retina/scale", &mScale)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    interface->addParam(CreateFloatParam("retina/dialation", &mDialation)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    interface->addParam(CreateFloatParam("retina/dialationscale", &mDialationScale)
                         .minValue(1.0f)
                         .maxValue(4.0f)
                         .oscReceiver(mName));
    interface->addParam(CreateFloatParam("retina/pattamp", &mPatternAmp)
                         .minValue(0.001f)
                         .maxValue(0.5f)
                         .oscReceiver(mName));
    interface->addParam(CreateFloatParam("retina/pattfreq", &mPatternFreq)
                         .minValue(1.0f)
                         .maxValue(40.0f)
                         .oscReceiver(mName));
    
    interface->addEnum(CreateEnumParam("retain/dialation-band", &mDialationBand)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
//    interface->addEnum(CreateEnumParam("retain/pattern-band", &mPatternBand)
//                       .maxValue(bandNames.size())
//                       .isVertical()
//                       .oscReceiver(oscName)
//                       .sendFeedback(), bandNames);
}

void RetinaShader::setCustomParams(AudioInputHandler &audioInputHandler)
{
    float dialation = mDialation + mDialationScale * audioInputHandler.getAverageVolumeByBand(mDialationBand);
    
    mShader.uniform( "iDialation", dialation );
    mShader.uniform( "iDialationScale", mDialationScale );
    mShader.uniform( "iPatternAmp", mPatternAmp );
    mShader.uniform( "iPatternFreq", mPatternFreq );
    mShader.uniform( "iScale", mScale );
}

#pragma mark - BioFractal

BioFractalShader::BioFractalShader()
: FragShader("biofractal", "livingkifs_frag.glsl")
{
    mIterations = 25;
    mJulia = Vec3f(-2.f,-1.5f,-.5f);
    mRotation = Vec3f(0.5f,-0.05f,-0.5f);
    mLightDir = Vec3f(0.5f,1.f,0.5f);
    mScale = 1.27f;
    mRotAngle = 40.0f;
    mAmplitude = 0.45f;
    mDetail = 0.025f;
}

void BioFractalShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(mName);
    interface->addParam(CreateIntParam( "biofractal/iterations", &mIterations )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName()));
    interface->addParam(CreateVec3fParam("biofractal/rotation", mRotation.ptr(), Vec3f(-1.0f,-1.0f,-1.0f), Vec3f(1.0f,1.0f,1.0f))
                         .oscReceiver(mName));
    interface->addParam(CreateVec3fParam("biofractal/julia", mJulia.ptr(), Vec3f(-5.0f,-5.0f,-5.0f), Vec3f(5.0f,5.0f,5.0f))
                         .oscReceiver(mName));
    interface->addParam(CreateVec3fParam("biofractal/lightdir", &mLightDir, Vec3f(-1.0f,-1.0f,-1.0f), Vec3f(1.0f,1.0f,1.0f))
                         .oscReceiver(mName));
    interface->addParam(CreateFloatParam( "biofractal/scale", &mScale )
                         .minValue(0.0f)
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "biofractal/angle", &mRotAngle )
                         .minValue(0.0f)
                         .maxValue(90.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "biofractal/amplitude", &mAmplitude )
                         .minValue(0.0f)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "biofractal/detail", &mDetail )
                         .minValue(0.0f)
                         .maxValue(0.1f));
}

void BioFractalShader::setCustomParams(AudioInputHandler &audioInputHandler)
{
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iJulia", mJulia );
    mShader.uniform( "iRotation", mRotation );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iRotAngle", mRotAngle );
    mShader.uniform( "iAmplitude", mAmplitude );
    mShader.uniform( "iLightDir", mLightDir );
    mShader.uniform( "iDetail", mDetail );
}

#pragma mark - Gyroid

GyroidShader::GyroidShader()
: FragShader("gyroid", "gyroid_frag.glsl")
{
}

void GyroidShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + mName;
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(mName);
    
}

void GyroidShader::setCustomParams(AudioInputHandler &audioInputHandler)
{
}
