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

TextureShaders::TextureShaders()
: Scene("TextureShaders")
{
    //mAudioInputHandler.setup(true);
}

TextureShaders::~TextureShaders()
{
}

void TextureShaders::setup()
{
    Scene::setup();
    
    setupShaders();
    
    mShaderType = SHADER_CELLS;
    
    reset();
}

void TextureShaders::setupShaders()
{
#define SHADERS_ENTRY( nam, glsl, enm ) \
    mShaders.push_back( loadFragShader( glsl ) );
SHADERS_TUPLE
#undef SHADERS_ENTRY
    
    mCellsParams.mTimeStep1 = 1.0f;
    mCellsParams.mTimeStep2 = 0.5f;
    mCellsParams.mTimeStep3 = 0.25f;
    mCellsParams.mTimeStep4 = 0.125f;
    mCellsParams.mTimeStep5 = 0.125f;
    mCellsParams.mTimeStep6 = 0.065f;
    mCellsParams.mTimeStep7 = 0.0f;
    
}

void TextureShaders::reset()
{
    mElapsedTime = 0.0f;
}

void TextureShaders::setupInterface()
{
    vector<string> shaderNames;
#define SHADERS_ENTRY( nam, glsl, enm ) \
    shaderNames.push_back(nam);
SHADERS_TUPLE
#undef  SHADERS_ENTRY
    mInterface->addEnum(CreateEnumParam( "Shader", (int*)(&mShaderType) )
                        .maxValue(SHADERS_COUNT)
                        .oscReceiver(getName(), "shader")
                        .isVertical(), shaderNames);
    
    mInterface->addParam(CreateFloatParam( "TimeScale", &mTimeScale )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    // SHADER_CELLS
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("CELLS");
    mInterface->addParam(CreateFloatParam( "CellSize", &mCellsParams.mCellSize )
                         .maxValue(3.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "Highlight", &mCellsParams.mHighlight )
                         .maxValue(6.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep1", &mCellsParams.mTimeStep1 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep2", &mCellsParams.mTimeStep2 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep3", &mCellsParams.mTimeStep3 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep4", &mCellsParams.mTimeStep4 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep5", &mCellsParams.mTimeStep5 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep6", &mCellsParams.mTimeStep6 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "TimeStep7", &mCellsParams.mTimeStep7 )
                         .maxValue(2.0f)
                         .oscReceiver(getName()));
    
    //mAudioInputHandler.setupInterface(mInterface);
}

void TextureShaders::update(double dt)
{
    Scene::update(dt);
    
    //mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mElapsedTime += dt;
}

void TextureShaders::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void TextureShaders::shaderPreDraw()
{
    gl::GlslProg shader = mShaders[mShaderType];
    shader.bind();
    
    Vec3f resolution( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mApp->getElapsedSeconds() );
    
    switch (mShaderType) {
        case SHADER_CELLS:
            shader.uniform("iCellSize", mCellsParams.mCellSize);
            shader.uniform("iHighlight", mCellsParams.mHighlight);
            shader.uniform("iTimeStep1", mCellsParams.mTimeStep1);
            shader.uniform("iTimeStep2", mCellsParams.mTimeStep2);
            shader.uniform("iTimeStep3", mCellsParams.mTimeStep3);
            shader.uniform("iTimeStep4", mCellsParams.mTimeStep4);
            shader.uniform("iTimeStep5", mCellsParams.mTimeStep5);
            shader.uniform("iTimeStep6", mCellsParams.mTimeStep6);
            shader.uniform("iTimeStep7", mCellsParams.mTimeStep7);
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
}

void TextureShaders::drawScene()
{
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    shaderPreDraw();
    
    drawShaderOutput();
    
    shaderPostDraw();
    
    gl::popMatrices();
}

void TextureShaders::drawDebug()
{
    //mAudioInputHandler.drawDebug(mApp->getViewportSize());
}
