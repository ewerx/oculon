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
    mShaderFbo = gl::Fbo( mApp->getViewportWidth(), mApp->getViewportHeight(), format );
    
    setupShaders();
    
    // color maps
    mColorMapTexture[0] = gl::Texture( loadImage( loadResource( "colortex1.jpg" ) ) );
    mColorMapTexture[1] = gl::Texture( loadImage( loadResource( "colortex2.jpg" ) ) );
    mColorMapTexture[2] = gl::Texture( loadImage( loadResource( "colortex3.jpg" ) ) );
    mColorMapTexture[3] = gl::Texture( loadImage( loadResource( "colortex4.jpg" ) ) );
    mColorMapIndex = 0;
    
    mShaderType = SHADER_METAHEX;

    mDrawOnSphere = false;
    
    mAudioResponseFreqMin = 0.0f;
    mAudioResponseFreqMax = 1.0f;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    mColor3 = ColorA::white();
    
    mTimeScale = 1.0f;
    
    reset();
}

void ObjectShaders::setupShaders()
{
#define OS_SHADERS_ENTRY( nam, glsl, enm ) \
    mShaders.push_back( loadFragShader( glsl ) );
OS_SHADERS_TUPLE
#undef OS_SHADERS_ENTRY
    
    // metahex
    mMetaHexParams.mQuality = 4;
    mMetaHexParams.mRenderSteps = 33;
    mMetaHexParams.mLightSpeed = 1.0f;
    mMetaHexParams.mSpeed = 1.0f;
    mMetaHexParams.mNumObjects = 9;
    mMetaHexParams.mCoeffecients = Vec3f(0.967f,0.423f,0.76321f);
    mMetaHexParams.mAudioCoeffs = false;
    
    // tilings
    mTilingsParams.mIterations = 20;
    mTilingsParams.mAngleP = 3;
    mTilingsParams.mAngleQ = 5;
    mTilingsParams.mAngleR = 2;
    mTilingsParams.mThickness = 0.03f;
    
    // retina
    mRetinaParams.mDialation = 0.2f;
    mRetinaParams.mDialationScale = 1.0f;
    mRetinaParams.mPatternAmp = 0.05f;
    mRetinaParams.mPatternFreq = 20.0f;
    mRetinaParams.mAudioPattern = false;
}

void ObjectShaders::reset()
{
    mElapsedTime = 0.0f;
}

void ObjectShaders::setupInterface()
{
    vector<string> shaderNames;
#define OS_SHADERS_ENTRY( nam, glsl, enm ) \
    shaderNames.push_back(nam);
OS_SHADERS_TUPLE
#undef  OS_SHADERS_ENTRY
    mInterface->addEnum(CreateEnumParam( "shader", (int*)(&mShaderType) )
                        .maxValue(SHADERS_COUNT)
                        .oscReceiver(getName(), "shader")
                        .isVertical(), shaderNames);
    
    mInterface->addParam(CreateFloatParam( "timeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color3", &mColor3, kMinColor, kMaxColor));
    
    mInterface->addParam(CreateIntParam( "colormap", &mColorMapIndex )
                         .maxValue(MAX_COLORMAPS-1)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateFloatParam("freqmin", &mAudioResponseFreqMin)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 20));
    mInterface->addParam(CreateFloatParam("freqmax", &mAudioResponseFreqMax)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 21));
    
    // SHADER_METAHEX
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("metahex");
    mInterface->addParam(CreateIntParam( "metahex/objects", &mMetaHexParams.mNumObjects )
                         .maxValue(24)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "metahex/steps", &mMetaHexParams.mRenderSteps )
                         .maxValue(128)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "metahex/quality", &mMetaHexParams.mQuality )
                         .maxValue(16)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "metahex/speed", &mMetaHexParams.mSpeed )
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "metahex/lightspeed", &mMetaHexParams.mLightSpeed )
                         .maxValue(10.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("metahex/coeffs", &mMetaHexParams.mCoeffecients, Vec3f::zero(), Vec3f(1.0f,1.0f,1.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateBoolParam("metahex/audiocoeffs", &mMetaHexParams.mAudioCoeffs )
                         .oscReceiver(mName));
    
    // SHADER_TILINGS
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("tilings");
    mInterface->addParam(CreateIntParam( "tilings/iterations", &mTilingsParams.mIterations )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "tilings/anglep", &mTilingsParams.mAngleP )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "tilings/angleq", &mTilingsParams.mAngleQ )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "tilings/angler", &mTilingsParams.mAngleR )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "tilings/thickness", &mTilingsParams.mThickness )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    
    // retina
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("retina");
    mInterface->addParam(CreateBoolParam("retina/audiodialation", &mRetinaParams.mAudioDialation )
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/dialation", &mRetinaParams.mDialation)
                         .maxValue(2.0f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/dialationscale", &mRetinaParams.mDialationScale)
                         .minValue(1.0f)
                         .maxValue(2.0f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/pattamp", &mRetinaParams.mPatternAmp)
                         .minValue(0.001f)
                         .maxValue(0.5f)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("retina/pattfreq", &mRetinaParams.mPatternFreq)
                         .minValue(1.0f)
                         .maxValue(40.0f)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateBoolParam("retina/audiopattern", &mRetinaParams.mAudioPattern )
                         .oscReceiver(mName));
    
    // audio input
    mApp->getAudioInputHandler().setupInterface(mInterface, mName);
}

void ObjectShaders::update(double dt)
{
    Scene::update(dt);
    
    mElapsedTime += dt;
    
    switch (mShaderType)
    {
        case SHADER_METAHEX:
            if (mMetaHexParams.mAudioCoeffs)
            {
                const float lows = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.0f, 0.25f) * mGain;
                const float mids = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.25f, 0.75f) * mGain;
                const float highs = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.75f, 1.0f) * mGain;
                
                mMetaHexParams.mCoeffecients.z = 0.5f + lows;
                mMetaHexParams.mCoeffecients.y = 0.5f + mids*3.0f;
                mMetaHexParams.mCoeffecients.x = 0.5f + highs*2.0f;
            }
            break;
            
        case SHADER_RETINA:
            if (mRetinaParams.mAudioDialation)
            {
                const float lows = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.0f, 0.25f) * mGain;
                mRetinaParams.mDialation = 0.5f + lows;
            }
            if (mRetinaParams.mAudioPattern)
            {
                const float mids = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.25f, 0.75f) * mGain;
                const float highs = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(0.75f, 1.0f) * mGain;
                
                mRetinaParams.mPatternAmp = 0.01f + 0.25f*mids;
                mRetinaParams.mPatternFreq = 5.0f + 10.0f*highs;
            }
            break;
            
        default:
            break;
    }
    
    if (mDrawOnSphere)
    {
        gl::pushMatrices();
        mShaderFbo.bindFramebuffer();
        gl::setMatricesWindow( mShaderFbo.getSize(), false );
        gl::setViewport( mShaderFbo.getBounds() );
        gl::enableDepthWrite();
        shaderPreDraw();
        drawShaderOutput();
        shaderPostDraw();
        mShaderFbo.unbindFramebuffer();
        mShaderFbo.getTexture().setWrap( GL_REPEAT, GL_REPEAT );
        gl::popMatrices();
    }
}

void ObjectShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void ObjectShaders::shaderPreDraw()
{
    mColorMapTexture[mColorMapIndex].bind(0);
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().bindTexture(1);
    }
    
    gl::GlslProg shader = mShaders[mShaderType];
    shader.bind();
    
    Vec3f resolution( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mElapsedTime );
    shader.uniform( "iTimeScale", mTimeScale);
    shader.uniform( "iColor1", mColor1);
    shader.uniform( "iColor2", mColor2);
    shader.uniform( "iColor3", mColor3);
    shader.uniform( "iBackgroundAlpha", mBackgroundAlpha);
    
    switch (mShaderType) {
        case SHADER_METAHEX:
            shader.uniform( "iObjSpeed", mMetaHexParams.mSpeed );
            shader.uniform( "iLightSpeed", mMetaHexParams.mLightSpeed );
            shader.uniform( "iNumObjects", mMetaHexParams.mNumObjects );
            shader.uniform( "iRenderSteps", mMetaHexParams.mRenderSteps );
            shader.uniform( "iQuality", mMetaHexParams.mQuality );
            shader.uniform( "iCoefficients", mMetaHexParams.mCoeffecients );
            break;
            
        case SHADER_TILINGS:
            shader.uniform( "iIterations", mTilingsParams.mIterations );
            shader.uniform( "iAngleP", mTilingsParams.mAngleP );
            shader.uniform( "iAngleQ", mTilingsParams.mAngleQ );
            shader.uniform( "iAngleR", mTilingsParams.mAngleR );
            shader.uniform( "iThickness", mTilingsParams.mThickness );
            break;
            
        case SHADER_RETINA:
            shader.uniform( "iDialation", mRetinaParams.mDialation );
            shader.uniform( "iDialationScale", mRetinaParams.mDialationScale );
            shader.uniform( "iPatternAmp", mRetinaParams.mPatternAmp );
            shader.uniform( "iPatternFreq", mRetinaParams.mPatternFreq );
            break;
            
        default:
            break;
    }
}

void ObjectShaders::drawShaderOutput()
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

void ObjectShaders::shaderPostDraw()
{
    mShaders[mShaderType].unbind();
    
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
    mColorMapTexture[mColorMapIndex].unbind();
}

void ObjectShaders::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    if (mDrawOnSphere)
    {
        gl::enableAlphaBlending();
        //gl::enable( GL_TEXTURE_2D );
        gl::enableDepthRead();
        gl::enableDepthWrite();
        gl::setMatrices( getCamera() );
        gl::setViewport( mApp->getViewportBounds() );
        gl::color( ColorA::white() );
        mShaderFbo.bindTexture();
        
        glEnable( GL_POLYGON_SMOOTH );
        //glEnable( GL_LIGHTING );
        //glEnable( GL_LIGHT0 );
        
        //GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        //glLightfv( GL_LIGHT0, GL_POSITION, light_position );
        
        gl::drawSphere(Vec3f::zero(), 640.0f, 64);
        //gl::drawSolidRect(mApp->getViewportBounds());
        
        mShaderFbo.unbindTexture();
        
        //glDisable( GL_LIGHT0 );
        //glDisable( GL_LIGHTING );
    }
    else
    {
        shaderPreDraw();
        drawShaderOutput();
        shaderPostDraw();
    }
    
    gl::popMatrices();
}

void ObjectShaders::drawDebug()
{
    //mApp->getAudioInputHandler().drawDebug(mApp->getViewportSize());
    
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( mApp->getWindowSize() );
    
    const Vec2f size(128,72);
    Rectf preview( 100.0f, size.y - 200.0f, 180.0f, size.y - 120.0f );
    gl::draw( mShaderFbo.getTexture(), mShaderFbo.getBounds(), preview );
    
    gl::disable( GL_TEXTURE_2D );
}
