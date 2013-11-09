//
//  TextureShaders.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "TextureShaders.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"

using namespace ci;
using namespace ci::app;
using namespace std;

TextureShaders::TextureShaders()
: Scene("textureshaders")
{
    //mAudioInputHandler.setup(true);
}

TextureShaders::~TextureShaders()
{
}

void TextureShaders::setup()
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
    
    mShaderType = SHADER_CELLS;

    mDrawOnSphere = false;
    
    mAudioResponseFreqMin = 0.0f;
    mAudioResponseFreqMax = 1.0f;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    
    reset();
}

void TextureShaders::setupShaders()
{
#define TS_SHADERS_ENTRY( nam, glsl, enm ) \
    mShaders.push_back( loadFragShader( glsl ) );
TS_SHADERS_TUPLE
#undef TS_SHADERS_ENTRY
    
    // noise
    mNoiseParams.mNoiseScale            = Vec3f(1.0f,1.0f,0.25f);
    mNoiseParams.mDisplacementSpeed     = 1.0f;
    mNoiseParams.mLevels                = 64.0f;
    mNoiseParams.mEdgeThickness         = 0.0f;
    mNoiseParams.mBrightness            = 1.0f;
    
    // cells
    mCellsParams.mHighlightAudioResponse = true;
    mCellsParams.mZoom = 1.0f;
    mCellsParams.mHighlight = 0.6f;
    mCellsParams.mIntensity = 1.0f;
    
    mCellsParams.mTimeStep[0] = 1.0f;
    mCellsParams.mTimeStep[1] = 0.5f;
    mCellsParams.mTimeStep[2] = 0.25f;
    mCellsParams.mTimeStep[3] = 0.125f;
    mCellsParams.mTimeStep[4] = 0.125f;
    mCellsParams.mTimeStep[5] = 0.065f;
    mCellsParams.mTimeStep[6] = 0.0f;
    
    mCellsParams.mFrequency[0] = 1.0f;
    mCellsParams.mFrequency[1] = 2.0f;
    mCellsParams.mFrequency[2] = 4.0f;
    mCellsParams.mFrequency[3] = 8.0f;
    mCellsParams.mFrequency[4] = 32.0f;
    mCellsParams.mFrequency[5] = 64.0f;
    mCellsParams.mFrequency[6] = 128.0f;
    
    // kali
    mKaliParams.iterations=20;
    mKaliParams.scale=1.35f;
    mKaliParams.fold= Vec2f(0.5f,0.5f);
    mKaliParams.translate= Vec2f(1.5f,1.5f);
    mKaliParams.zoom=0.17f;
    mKaliParams.brightness=7.f;
    mKaliParams.saturation=0.2f;
    mKaliParams.texturescale=0.15f;
    mKaliParams.rotspeed=0.005f;
    mKaliParams.colspeed=0.05f;
    mKaliParams.antialias=2.f;
    
    // simplicity
    mSimplicityParams.mRedPower = 2;
    mSimplicityParams.mGreenPower = 1;
    mSimplicityParams.mBluePower = 0;
    mSimplicityParams.mColorScale = Vec3f(1.8f, 1.4f, 1.0f);
    mSimplicityParams.mStrengthFactor = 0.03f;
    mSimplicityParams.mStrengthMin = 7.0f;
    mSimplicityParams.mStrengthConst = 4373.11f;
    mSimplicityParams.mIterations = 32;
    mSimplicityParams.mAccumPower = 2.3f;
    mSimplicityParams.mMagnitude = Vec3f(-0.5f, -0.4f, -1.5f);
    mSimplicityParams.mFieldScale = 5.0f;
    mSimplicityParams.mFieldSubtract = 0.7f;
    mSimplicityParams.mTimeScale = 1.0f;
    mSimplicityParams.mPanSpeed = Vec3f(0.0625f, 0.0833f, 0.0078f);
    mSimplicityParams.mUVOffset = Vec3f(1.0f, -1.3f, 0.0f);
    mSimplicityParams.mUVScale = 0.25f;
}

void TextureShaders::reset()
{
    mElapsedTime = 0.0f;
    
    for (int i = 0; i < tCellsParams::CELLS_NUM_LAYERS; ++i)
    {
        mCellsParams.mTime[i] = 0.0f;
    }
}

void TextureShaders::setupInterface()
{
    vector<string> shaderNames;
#define TS_SHADERS_ENTRY( nam, glsl, enm ) \
    shaderNames.push_back(nam);
TS_SHADERS_TUPLE
#undef  TS_SHADERS_ENTRY
    mInterface->addEnum(CreateEnumParam( "Shader", (int*)(&mShaderType) )
                        .maxValue(SHADERS_COUNT)
                        .oscReceiver(getName(), "shader")
                        .isVertical(), shaderNames);
    
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    mInterface->addParam(CreateIntParam( "colormap", &mColorMapIndex )
                         .maxValue(MAX_COLORMAPS-1)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateFloatParam("color1alpha", &(mColor1.a))
                         .oscReceiver(getName())
                         .midiInput(1, 2, 22));
    mInterface->addParam(CreateFloatParam("gain", &mGain)
                         .maxValue(20.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 23));
    mInterface->addParam(CreateFloatParam("freqmin", &mAudioResponseFreqMin)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 20));
    mInterface->addParam(CreateFloatParam("freqmax", &mAudioResponseFreqMax)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 21));
    
    // SHADER_CELLS
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Cells");
    mInterface->addParam(CreateFloatParam( "Zoom", &mCellsParams.mZoom )
                         .minValue(0.01f)
                         .maxValue(3.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 16));
    mInterface->addParam(CreateBoolParam( "HighlightByAudio", &mCellsParams.mHighlightAudioResponse )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Highlight", &mCellsParams.mHighlight )
                         .maxValue(6.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 17));
    mInterface->addParam(CreateFloatParam( "Intensity", &mCellsParams.mIntensity )
                         .minValue(1.0f)
                         .maxValue(8.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep1", &mCellsParams.mTimeStep[0] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 18));
    mInterface->addParam(CreateFloatParam( "TimeStep2", &mCellsParams.mTimeStep[1] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 19));
    mInterface->addParam(CreateFloatParam( "TimeStep3", &mCellsParams.mTimeStep[2] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep4", &mCellsParams.mTimeStep[3] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep5", &mCellsParams.mTimeStep[4] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep6", &mCellsParams.mTimeStep[5] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep7", &mCellsParams.mTimeStep[6] )
                         .minValue(-2.0f)
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency1", &mCellsParams.mFrequency[0] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency2", &mCellsParams.mFrequency[1] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency3", &mCellsParams.mFrequency[2] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency4", &mCellsParams.mFrequency[3] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency5", &mCellsParams.mFrequency[4] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency6", &mCellsParams.mFrequency[5] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Frequency7", &mCellsParams.mFrequency[6] )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    
    // SHADER_KALI
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Kali");
    mInterface->addParam(CreateIntParam( "kali/iterations", &mKaliParams.iterations )
                         .maxValue(64)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec2fParam("kali/fold", &mKaliParams.fold, Vec2f::zero(), Vec2f(1.0f,1.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateVec2fParam("kali/translate", &mKaliParams.translate, Vec2f::zero(), Vec2f(5.0f,5.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "kali/scale", &mKaliParams.scale )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/zoom", &mKaliParams.zoom )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    //.midiInput(0, 2, 19));
    mInterface->addParam(CreateFloatParam( "kali/brightness", &mKaliParams.brightness )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/saturation", &mKaliParams.saturation )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/texturescale", &mKaliParams.texturescale )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/rotspeed", &mKaliParams.rotspeed )
                         .maxValue(0.1f)
                         .oscReceiver(getName()));
    //.midiInput(0, 2, 20));
    mInterface->addParam(CreateFloatParam( "kali/colspeed", &mKaliParams.colspeed )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "kali/antialias", &mKaliParams.antialias )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    

    // SHADER_SIMPLICITY
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Simplicity");
    mInterface->addParam(CreateIntParam( "Red Power", &mSimplicityParams.mRedPower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver("/1/fader2"));
    mInterface->addParam(CreateIntParam( "Green Power", &mSimplicityParams.mGreenPower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver("/1/fader2"));
    mInterface->addParam(CreateIntParam( "Blue Power", &mSimplicityParams.mBluePower )
                         .minValue(0)
                         .maxValue(2)
                         .oscReceiver("/1/fader2"));
    mInterface->addParam(CreateIntParam( "Iterations", &mSimplicityParams.mIterations )
                         .minValue(0)
                         .maxValue(64));
    mInterface->addParam(CreateFloatParam( "StrengthFactor", &mSimplicityParams.mStrengthFactor )
                         .minValue(0.0f)
                         .maxValue(1.0f));
    mInterface->addParam(CreateFloatParam( "StrengthMin", &mSimplicityParams.mStrengthMin )
                         .minValue(0.0f)
                         .maxValue(20.0f));
    mInterface->addParam(CreateFloatParam( "StrengthConst", &mSimplicityParams.mStrengthConst )
                         .minValue(4000.0f)
                         .maxValue(5000.0f));
    mInterface->addParam(CreateFloatParam( "AccumPower", &mSimplicityParams.mAccumPower )
                         .minValue(1.0f)
                         .maxValue(4.0f));
    mInterface->addParam(CreateFloatParam( "FieldScale", &mSimplicityParams.mFieldScale )
                         .minValue(1.0f)
                         .maxValue(10.0f));
    mInterface->addParam(CreateFloatParam( "FieldSubtract", &mSimplicityParams.mFieldSubtract )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateFloatParam( "TimeScale", &mSimplicityParams.mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateVec3fParam("magnitude", &mSimplicityParams.mMagnitude, Vec3f(-1.0f,-1.0f,-2.0f), Vec3f::zero()));
    mInterface->addParam(CreateVec3fParam("color_scale", &mSimplicityParams.mColorScale, Vec3f::zero(), Vec3f(3.0f,3.0f,3.0f)));
    mInterface->addParam(CreateVec3fParam("PanSpeed", &mSimplicityParams.mPanSpeed, Vec3f::zero(), Vec3f(0.1f,0.1f,0.1f)));
    mInterface->addParam(CreateVec3fParam("UVOffset", &mSimplicityParams.mUVOffset, Vec3f(-4.0f,-4.0f,-4.0f), Vec3f(4.0f,4.0f,4.0f)));
    mInterface->addParam(CreateFloatParam( "UVScale", &mSimplicityParams.mUVScale )
                         .minValue(0.01f)
                         .maxValue(4.0f));
    
    // SHADER_NOISE
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("Noise");
    mInterface->addParam(CreateFloatParam( "noise/speed", &mNoiseParams.mDisplacementSpeed )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateVec3fParam("noise/noise", &mNoiseParams.mNoiseScale, Vec3f::zero(), Vec3f(50.0f,50.0f,5.0f))
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam( "levels", &mNoiseParams.mLevels )
                         .maxValue(128.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "brightness", &mNoiseParams.mBrightness )
                         .oscReceiver(getName()));
    //.midiInput(0,2,16));
    
    mInterface->addParam(CreateFloatParam( "edgeThickness", &mNoiseParams.mEdgeThickness )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));

    
    // audio input
    mApp->getAudioInputHandler().setupInterface(mInterface, mName);
}

void TextureShaders::update(double dt)
{
    Scene::update(dt);
    
    mElapsedTime += dt;
    
    switch (mShaderType)
    {
        case SHADER_CELLS:
            if (mCellsParams.mHighlightAudioResponse)
            {
                const float midHighVolume = mApp->getAudioInputHandler().getAverageVolumeByFrequencyRange(mAudioResponseFreqMin, mAudioResponseFreqMax);
                mCellsParams.mHighlight = 100.0f * mGain * midHighVolume;
            }
            for (int i = 0; i < tCellsParams::CELLS_NUM_LAYERS; ++i)
            {
                mCellsParams.mTime[i] += dt * mCellsParams.mTimeStep[i];
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

void TextureShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void TextureShaders::shaderPreDraw()
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
    shader.uniform( "iColor1", mColor1);
    shader.uniform( "iColor2", mColor2);
    
    switch (mShaderType) {
        case SHADER_CELLS:
            shader.uniform("iZoom", mCellsParams.mZoom);
            shader.uniform("iHighlight", mCellsParams.mHighlight);
            shader.uniform("iTimeStep1", mCellsParams.mTime[0]);
            shader.uniform("iTimeStep2", mCellsParams.mTime[1]);
            shader.uniform("iTimeStep3", mCellsParams.mTime[2]);
            shader.uniform("iTimeStep4", mCellsParams.mTime[3]);
            shader.uniform("iTimeStep5", mCellsParams.mTime[4]);
            shader.uniform("iTimeStep6", mCellsParams.mTime[5]);
            shader.uniform("iTimeStep7", mCellsParams.mTime[6]);
            
            shader.uniform("iFrequency1", mCellsParams.mFrequency[0]);
            shader.uniform("iFrequency2", mCellsParams.mFrequency[1]);
            shader.uniform("iFrequency3", mCellsParams.mFrequency[2]);
            shader.uniform("iFrequency4", mCellsParams.mFrequency[3]);
            shader.uniform("iFrequency5", mCellsParams.mFrequency[4]);
            shader.uniform("iFrequency6", mCellsParams.mFrequency[5]);
            shader.uniform("iFrequency7", mCellsParams.mFrequency[6]);
            shader.uniform("iIntensity", mCellsParams.mIntensity);
            break;
        case SHADER_KALI:
            shader.uniform( "iChannel0", 0 );
            shader.uniform( "iterations", mKaliParams.iterations );
            shader.uniform( "scale", mKaliParams.scale );
            shader.uniform( "fold", mKaliParams.fold );
            shader.uniform( "translate", mKaliParams.translate );
            shader.uniform( "zoom", mKaliParams.zoom );
            shader.uniform( "brightness", mKaliParams.brightness );
            shader.uniform( "saturation", mKaliParams.saturation );
            shader.uniform( "texturescale", mKaliParams.texturescale );
            shader.uniform( "rotspeed", mKaliParams.rotspeed );
            shader.uniform( "colspeed", mKaliParams.colspeed );
            shader.uniform( "antialias", mKaliParams.antialias );
            break;
            
        case SHADER_NOISE:
            shader.uniform( "theta", (float)(mElapsedTime * mNoiseParams.mDisplacementSpeed) );
            shader.uniform( "scale", mNoiseParams.mNoiseScale );
            shader.uniform( "colorScale", mColor1 );
            shader.uniform( "alpha", mNoiseParams.mBrightness );
            shader.uniform( "levels", mNoiseParams.mLevels );
            shader.uniform( "edgeThickness", mNoiseParams.mEdgeThickness );
            break;
            
        case SHADER_SIMPLICITY:
            shader.uniform( "colorScale", mSimplicityParams.mColorScale );
            shader.uniform( "rPower", mSimplicityParams.mRedPower );
            shader.uniform( "gPower", mSimplicityParams.mGreenPower );
            shader.uniform( "bPower", mSimplicityParams.mBluePower );
            shader.uniform( "strengthFactor", mSimplicityParams.mStrengthFactor );
            shader.uniform( "strengthMin", mSimplicityParams.mStrengthMin );
            shader.uniform( "strengthConst", mSimplicityParams.mStrengthConst );
            shader.uniform( "iterations", mSimplicityParams.mIterations );
            shader.uniform( "accumPower", mSimplicityParams.mAccumPower );
            shader.uniform( "magnitude", mSimplicityParams.mMagnitude );
            shader.uniform( "fieldScale", mSimplicityParams.mFieldScale );
            shader.uniform( "fieldSubtract", mSimplicityParams.mFieldSubtract );
            shader.uniform( "timeScale", mSimplicityParams.mTimeScale );
            shader.uniform( "panSpeed", mSimplicityParams.mPanSpeed );
            shader.uniform( "uvOffset", mSimplicityParams.mUVOffset );
            shader.uniform( "uvScale", mSimplicityParams.mUVScale );
            break;
            

            
        default:
            break;
    }
}

void TextureShaders::drawShaderOutput()
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

void TextureShaders::shaderPostDraw()
{
    mShaders[mShaderType].unbind();
    
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
    mColorMapTexture[mColorMapIndex].unbind();
}

void TextureShaders::drawScene()
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

void TextureShaders::drawDebug()
{
    //mApp->getAudioInputHandler().drawDebug(mApp->getViewportSize());
    
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( mApp->getWindowSize() );
    
    const Vec2f size(128,72);
    Rectf preview( 100.0f, size.y - 200.0f, 180.0f, size.y - 120.0f );
    gl::draw( mShaderFbo.getTexture(), mShaderFbo.getBounds(), preview );
    
    gl::disable( GL_TEXTURE_2D );
}
