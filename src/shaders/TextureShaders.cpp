//
//  TextureShaders.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "TextureShaders.h"
#include "Interface.h"
#include "Utils.h"

#include "cinder/Utilities.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

TextureShaders::TextureShaders(const std::string& name)
: Scene(name)
{
}

TextureShaders::~TextureShaders()
{
}

void TextureShaders::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(true);
    
    setupShaders();
    
    // color maps
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex1.jpg" ) ) ) ) );
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex2.jpg" ) ) ) ) );
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex3.jpg" ) ) ) ) );
    mColorMaps.push_back(make_pair("colormap1", gl::Texture( loadImage( loadResource( "colortex4.jpg" ) ) ) ) );
    mColorMapIndex = 0;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    
    reset();
}

void TextureShaders::setupShaders()
{
    mShaderType = 0;
    
    mShaders.push_back( new BezierShader() );
    mShaders.push_back( new SimplicityShader() );
    mShaders.push_back( new KifsShader() );
    mShaders.push_back( new PixelWeaveShader() );
}

void TextureShaders::reset()
{
}

void TextureShaders::setupInterface()
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
    
    mTimeController.setupInterface(mInterface, getName());
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    vector<string> colorMapNames;
    for( tNamedTexture namedTex : mColorMaps )
    {
        colorMapNames.push_back(namedTex.first);
    }
    mInterface->addEnum(CreateEnumParam( "colormap", (int*)(&mColorMapIndex) )
                        .maxValue(colorMapNames.size())
                        .oscReceiver(getName())
                        .isVertical(), colorMapNames);
    
    // custom params
    for( FragShader* shader : mShaders )
    {
        if (shader)
        {
            mInterface->gui()->addColumn();
            shader->setupInterface(mInterface, getName());
        }
    }

    // audio input
    mAudioInputHandler.setupInterface(mInterface, getName());
}

void TextureShaders::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    for( FragShader* shader : mShaders )
    {
        if (shader)
        {
            shader->update(mTimeController.getDelta());
        }
    }
}

void TextureShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void TextureShaders::shaderPreDraw()
{
    mColorMaps[mColorMapIndex].second.bind(0);
    if( mAudioInputHandler.hasTexture() )
    {
        mAudioInputHandler.getFbo().bindTexture(1);
    }
    
    gl::GlslProg shader = mShaders[mShaderType]->getShader();
    shader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    shader.uniform( "iColor1", mColor1);
    shader.uniform( "iColor2", mColor2);
    shader.uniform( "iChannel0", 0 );
    shader.uniform( "iChannel1", 1 );
    
    mShaders[mShaderType]->setCustomParams( mAudioInputHandler );
}

void TextureShaders::shaderPostDraw()
{
    gl::GlslProg shader = mShaders[mShaderType]->getShader();
    shader.unbind();
    
    if( mAudioInputHandler.hasTexture() )
    {
        mAudioInputHandler.getFbo().unbindTexture();
    }
    mColorMaps[mColorMapIndex].second.unbind();
}

void TextureShaders::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    Utils::drawTexturedRect( mApp->getViewportBounds() );
    shaderPostDraw();
    
    gl::popMatrices();
}

//void TextureShaders::drawDebug()
//{
//    //mAudioInputHandler.drawDebug(mApp->getViewportSize());
//    
//    gl::enable( GL_TEXTURE_2D );
//    gl::setMatricesWindow( mApp->getWindowSize() );
//    
//    const Vec2f size(128,72);
//    Rectf preview( 100.0f, size.y - 200.0f, 180.0f, size.y - 120.0f );
//    gl::draw( mShaderFbo.getTexture(), mShaderFbo.getBounds(), preview );
//    
//    gl::disable( GL_TEXTURE_2D );
//}

#pragma mark - KIFS

TextureShaders::KifsShader::KifsShader()
: FragShader("kifs", "kifs_frag.glsl")
{
    iterations=20;
    scale=1.35f;
    fold= Vec2f(0.5f,0.5f);
    translate= Vec2f(1.5f,1.5f);
    zoom=0.17f;
    brightness=7.f;
    saturation=0.2f;
    texturescale=0.15f;
    rotspeed=0.005f;
    colspeed=0.05f;
    antialias=2.f;
    mRotationOffset = 0.0f;
    mRotation = 0.0f;
    mColorOffset = 0.0f;
    mAudioFold = false;
    mAudioTranslate = false;
    mAudioRot = false;
}

void TextureShaders::KifsShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateIntParam( "kali/iterations", &iterations )
                         .maxValue(64)
                         .oscReceiver(oscName));
    interface->addParam(CreateVec2fParam("kali/fold", &fold, Vec2f::zero(), Vec2f(1.0f,1.0f))
                         .oscReceiver(getName()));
    interface->addParam(CreateVec2fParam("kali/translate", &translate, Vec2f::zero(), Vec2f(5.0f,5.0f))
                         .oscReceiver(getName()));
    interface->addParam(CreateFloatParam( "kali/scale", &scale )
                         .maxValue(3.0f)
                         .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "kali/zoom", &zoom )
                         .maxValue(1.0f)
                         .oscReceiver(oscName));
    //.midiInput(0, 2, 19));
    interface->addParam(CreateFloatParam( "kali/brightness", &brightness )
                         .maxValue(20.0f)
                         .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "kali/saturation", &saturation )
                         .maxValue(2.0f)
                         .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "kali/texturescale", &texturescale )
                         .maxValue(0.2f)
                         .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "kali/rotspeed", &rotspeed )
                         .maxValue(1.0f)
                         .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "kali/rotoffset", &mRotationOffset )
                         .maxValue(5.0f)
                         .oscReceiver(oscName));
    //.midiInput(0, 2, 20));
    interface->addParam(CreateFloatParam( "kali/colspeed", &colspeed )
                         .maxValue(1.0f)
                         .oscReceiver(oscName));
    interface->addParam(CreateFloatParam( "kali/antialias", &antialias )
                         .maxValue(3.0f)
                         .oscReceiver(oscName));
    
    interface->addParam(CreateBoolParam("kali/audiofold", &mAudioFold)
                         .oscReceiver(oscName));
    interface->addParam(CreateBoolParam("kali/audiotrans", &mAudioTranslate)
                         .oscReceiver(oscName));
    interface->addParam(CreateBoolParam("kali/audiorot", &mAudioRot)
                         .oscReceiver(oscName));
}

void TextureShaders::KifsShader::update(double dt)
{
    mRotation += rotspeed * dt;
    mColorOffset += colspeed * dt;
}

void TextureShaders::KifsShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    Vec2f fold = fold;
    if (mAudioFold)
    {
        fold.x *= audioInputHandler.getAverageVolumeMidFreq() * 5.0f;
        fold.y *= audioInputHandler.getAverageVolumeLowFreq() * 5.0f;
    }
    
    Vec2f translate = translate;
    if (mAudioFold)
    {
        translate.x *= audioInputHandler.getAverageVolumeMidFreq() * 5.0f;
        translate.y *= audioInputHandler.getAverageVolumeLowFreq() * 5.0f;
    }
    
    float rotationOffset = mRotationOffset;
    if (mAudioRot)
    {
        rotationOffset *= audioInputHandler.getAverageVolumeHighFreq() * 5.0f;
    }
    
    mShader.uniform( "iterations", iterations );
    mShader.uniform( "scale", scale );
    mShader.uniform( "fold", fold );
    mShader.uniform( "translate", translate );
    mShader.uniform( "zoom", zoom );
    mShader.uniform( "brightness", brightness );
    mShader.uniform( "saturation", saturation );
    mShader.uniform( "texturescale", texturescale );
    mShader.uniform( "rotation", mRotation + rotationOffset );
    mShader.uniform( "coloroffset", mColorOffset );
    mShader.uniform( "antialias", antialias );
}

#pragma mark - Simplicity

TextureShaders::SimplicityShader::SimplicityShader()
: FragShader("simplicity", "simplicity_frag.glsl")
{
    mRedPower = 2;
    mGreenPower = 1;
    mBluePower = 0;
    mColorScale = Vec3f(1.8f, 1.4f, 1.0f);
    mStrengthFactor = 0.03f;
    mStrengthMin = 7.0f;
    mStrengthConst = 4373.11f;
    mIterations = 32;
    mAccumPower = 2.3f;
    mMagnitude = Vec3f(-0.5f, -0.4f, -1.5f);
    mFieldScale = 5.0f;
    mFieldSubtract = 0.7f;
    mTimeScale = 1.0f;
    mPanSpeed = Vec3f(0.0f, 0.0f, 0.01f);
    mPanPos = Vec3f(0.0625f, 0.0833f, 0.0078f);
    mUVOffset = Vec3f(1.0f, -1.3f, 0.0f);
    mUVScale = 0.25f;
    mAudioHighlight = false;
    mAudioShift = false;
}

void TextureShaders::SimplicityShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    interface->addParam(CreateIntParam( "Red Power", &mRedPower )
                         .minValue(0)
                         .maxValue(2));
    interface->addParam(CreateIntParam( "Green Power", &mGreenPower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "Blue Power", &mBluePower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver(oscName));
    interface->addParam(CreateIntParam( "Iterations", &mIterations )
                         .minValue(0)
                         .maxValue(64));
    interface->addParam(CreateFloatParam( "StrengthFactor", &mStrengthFactor )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    interface->addParam(CreateFloatParam( "StrengthMin", &mStrengthMin )
                         .minValue(0.0f)
                         .maxValue(20.0f));
    interface->addParam(CreateFloatParam( "StrengthConst", &mStrengthConst )
                         .minValue(4000.0f)
                         .maxValue(5000.0f));
    interface->addParam(CreateFloatParam( "AccumPower", &mAccumPower )
                         .minValue(1.0f)
                         .maxValue(4.0f));
    interface->addParam(CreateFloatParam( "FieldScale", &mFieldScale )
                         .minValue(1.0f)
                         .maxValue(10.0f));
    interface->addParam(CreateFloatParam( "FieldSubtract", &mFieldSubtract )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    interface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    interface->addParam(CreateVec3fParam("magnitude", &mMagnitude, Vec3f(-1.0f,-1.0f,-2.0f), Vec3f::zero()));
    interface->addParam(CreateVec3fParam("color_scale", &mColorScale, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    interface->addParam(CreateVec3fParam("PanSpeed", &mPanSpeed, Vec3f(-0.2f,-0.2f,-0.2f), Vec3f(0.2f,0.2f,0.2f)));
    interface->addParam(CreateVec3fParam("UVOffset", &mUVOffset, Vec3f(-4.0f,-4.0f,-4.0f), Vec3f(4.0f,4.0f,4.0f)));
    interface->addParam(CreateFloatParam( "UVScale", mUVScale.ptr() )
                         .minValue(0.01f)
                         .maxValue(4.0f));
}

void TextureShaders::SimplicityShader::update(double dt)
{
    mPanPos = mPanPos() + (dt * mPanSpeed);
}

void TextureShaders::SimplicityShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "colorScale", mColorScale );
    mShader.uniform( "rPower", mRedPower );
    mShader.uniform( "gPower", mGreenPower );
    mShader.uniform( "bPower", mBluePower );
    mShader.uniform( "strengthFactor", mStrengthFactor );
    mShader.uniform( "strengthMin", mStrengthMin );
    mShader.uniform( "strengthConst", mStrengthConst );
    mShader.uniform( "iterations", mIterations );
    mShader.uniform( "accumPower", mAccumPower );
    mShader.uniform( "magnitude", mMagnitude );
    mShader.uniform( "fieldScale", mFieldScale );
    mShader.uniform( "fieldSubtract", mFieldSubtract );
    mShader.uniform( "timeScale", mTimeScale );
    mShader.uniform( "panPos", mPanPos() );
    mShader.uniform( "uvOffset", mUVOffset );
    mShader.uniform( "uvScale", mUVScale );
}

#pragma mark - Bezier

TextureShaders::BezierShader::BezierShader()
: FragShader("bezier", "bezier_frag.glsl")
{
    mSamples = 200;
    mThickness = 0.1f;
    mBlur = 20.0f;
    mFrequency = 200.0f;
    mPoint1Range = 0.5f;
    mPoint2Range = 0.5f;
    mBlurResponseBand = AudioInputHandler::BAND_NONE;
}

void TextureShaders::BezierShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam( "samples", &mSamples )
                        .minValue(0)
                        .maxValue(512));
    interface->addParam(CreateFloatParam( "thickness", &mThickness )
                        .minValue(0.01f)
                        .maxValue(5.0f));
    interface->addParam(CreateFloatParam( "blur", &mBlur )
                        .minValue(0.01f)
                        .maxValue(100.0f));
    interface->addEnum(CreateEnumParam("blur-response", &mBlurResponseBand)
                       .maxValue(bandNames.size())
                       .isVertical()
                       .oscReceiver(oscName)
                       .sendFeedback(), bandNames);
    interface->addParam(CreateFloatParam( "frequency", &mFrequency )
                        .minValue(1.0f)
                        .maxValue(2000.0f));
    interface->addParam(CreateFloatParam( "point1-range", &mPoint1Range )
                        .minValue(0.01f)
                        .maxValue(5.0f));
    interface->addParam(CreateFloatParam( "point2-range", &mPoint2Range )
                        .minValue(0.01f)
                        .maxValue(5.0f));
}

void TextureShaders::BezierShader::update(double dt)
{
}

void TextureShaders::BezierShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    float blur = (mBlur * 0.5f) + mBlur * audioInputHandler.getAverageVolumeByBand(mBlurResponseBand);
    mShader.uniform( "iSamples", mSamples );
    mShader.uniform( "iThickness", mThickness );
    mShader.uniform( "iBlur", blur );
    mShader.uniform( "iFrequency", mFrequency );
    mShader.uniform( "iPoint1Range", mPoint1Range );
    mShader.uniform( "iPoint2Range", mPoint2Range );
}

#pragma mark - GravityField

TextureShaders::GravityFieldShader::GravityFieldShader()
: FragShader("gravityfield", "gravityfield_frag.glsl")
{
    mPoints = 64;
    mMode = 0;
}

void TextureShaders::GravityFieldShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam( "points", &mPoints )
                        .minValue(1)
                        .maxValue(256));
    interface->addParam(CreateIntParam( "mode", &mMode )
                        .minValue(0)
                        .maxValue(2));
}

void TextureShaders::GravityFieldShader::update(double dt)
{
}

void TextureShaders::GravityFieldShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iMode", mMode );
    mShader.uniform( "iPoints", mPoints );
}

#pragma mark - PixelWeave

TextureShaders::PixelWeaveShader::PixelWeaveShader()
: FragShader("pixelweave", "pixelweave_frag.glsl")
{
    mBokeh = 1;
    mSpacing = 1;
}

void TextureShaders::PixelWeaveShader::setupInterface( Interface* interface, const std::string& prefix )
{
    string oscName = prefix + "/" + getName();
    vector<string> bandNames = AudioInputHandler::getBandNames();
    
    interface->gui()->addLabel(getName());
    
    interface->addParam(CreateIntParam( "bokeh", &mBokeh )
                        .minValue(1)
                        .maxValue(8));
    interface->addParam(CreateIntParam( "spacing", &mSpacing )
                        .minValue(0)
                        .maxValue(8));
}

void TextureShaders::PixelWeaveShader::setCustomParams( AudioInputHandler& audioInputHandler )
{
    mShader.uniform( "iBokehRad", mBokeh );
    mShader.uniform( "iTapSpacing", mSpacing );
}
