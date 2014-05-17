//
//  RootFract.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-13.
//
//

#include "RootFract.h"
#include "OculonApp.h"

using namespace ci;

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
    mColor1 = ColorAf(255.f/256.f,30.f/256.f,10.f/256.f);
    mColor2 = ColorAf(50.f/256.f,100.f/256.f,255.f/256.f);
    mColor3 = ColorAf(1.f/256.f,.9f/256.f,.75f/256.f);
    
    mIterations         = 60;
    mScale              = -0.3f;
    mSaturation         = 0.35f;
    mBrightness         = 0.9f;
    mContrast           = 4.0f;
    mMinBrightness      = 0.7f;
    mJulia              = Vec2f(1.8f, 0.26f);;
    mOrbitTraps         = Vec3f(.155f, .144f, 0.015f);
    mTrapWidths         = Vec3f(.01f, .03f, .3f);
    mTrapBrightness     = Vec3f(2.2f, 1.7f, 1.0f);
    mTrapContrast       = Vec3f(0.25f, 2.0f, 5.0f);
    
    
    mAudioInputHandler.setup(false);
    
    mShader = loadFragShader("rootfract_frag.glsl");
    
    reset();
}

void RootFract::reset()
{
    mTimeController.reset();
}

void RootFract::setupInterface()
{
    mTimeController.setupInterface(mInterface, mName);
    
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor).oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor).oscReceiver(mName));
    mInterface->addParam(CreateColorParam("color2", &mColor3, kMinColor, kMaxColor).oscReceiver(mName));
    
    mInterface->addParam(CreateIntParam( "iterations", &mIterations )
                         .maxValue(120));
    mInterface->addParam(CreateFloatParam("mScale", &mScale)
                         .minValue(-1.0f)
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
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
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateVec2fParam("julia", &mJulia, Vec2f::zero(), Vec2f(3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("traps", &mOrbitTraps, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("trap_width", &mTrapWidths, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("trap_bright", &mTrapBrightness, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("trap_contrast", &mTrapContrast, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    
    mAudioInputHandler.setupInterface(mInterface, mName);
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
    
    drawShaderOutput();
    
    shaderPostDraw();
    
    gl::popMatrices();
    
    gl::popMatrices();
}

void RootFract::shaderPreDraw()
{
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    mShader.uniform( "iColor1", mColor1);
    mShader.uniform( "iColor2", mColor2);
    mShader.uniform( "iColor3", mColor3);
    mShader.uniform( "iIterations", mIterations );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iSaturation", mSaturation );
    mShader.uniform( "iBrightness", mBrightness );
    mShader.uniform( "iContrast", mContrast );
    mShader.uniform( "iMinBrightness", mMinBrightness );
    mShader.uniform( "iJulia", mJulia );
    mShader.uniform( "iOrbitTraps", mOrbitTraps );
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

void RootFract::drawShaderOutput()
{
    // Draw shader output
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::begin( GL_TRIANGLES );
    
    // Define quad vertices
    const Area& bounds = mApp->getViewportBounds();
    
    Vec2f vert0( (float)bounds.x1, (float)bounds.y1 );
    Vec2f vert1( (float)bounds.x2, (float)bounds.y1 );
    Vec2f vert2( (float)bounds.x1, (float)bounds.y2 );
    Vec2f vert3( (float)bounds.x2, (float)bounds.y2 );
    
    // Define quad texture coordinates
    Vec2f uv0( 0.0f, 0.0f );
    Vec2f uv1( 1.0f, 0.0f );
    Vec2f uv2( 0.0f, 1.0f );
    Vec2f uv3( 1.0f, 1.0f );
    
    // Draw quad (two triangles)
    gl::texCoord( uv0 );
    gl::vertex( vert0 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    
    gl::texCoord( uv1 );
    gl::vertex( vert1 );
    gl::texCoord( uv2 );
    gl::vertex( vert2 );
    gl::texCoord( uv3 );
    gl::vertex( vert3 );
    
    gl::end();
}

void RootFract::shaderPostDraw()
{
    mShader.unbind();
}
