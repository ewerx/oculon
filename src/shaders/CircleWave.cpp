//
//  CircleWave.cpp
//  Oculon
//
//  Created by Ehsan on 13-10-24.
//  Copyright 2013 ewerx. All rights reserved.
//

#include "OculonApp.h"
#include "CircleWave.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include <boost/format.hpp>
#include "Interface.h"

using namespace ci;

CircleWave::CircleWave()
: Scene("circlewave")
{
    //mAudioInputHandler.setup(this, true);
}

CircleWave::~CircleWave()
{
}

void CircleWave::setup()
{
    Scene::setup();
    
    mShader = loadFragShader("circlewave_frag.glsl");
    
    reset();
}

void CircleWave::reset()
{
    mElapsedTime = 0.0f;
    
    mSeparation = 0.06f;
    mColorSeparation = true;
    mDetail = 0.5f;
    mStrands = 30;
    mScale = 0.75f;
    mThickness = 0.0625f;
    
    mColor1 = ColorA::white();
    mColor2 = ColorA::black();
    
//    mBackgroundReaction = BG_REACTION_NONE;
    mBackgroundFlash = false;
}

void CircleWave::setupInterface()
{
    mInterface->addParam(CreateColorParam("color1", &mColor1, kMinColor, kMaxColor));
    mInterface->addParam(CreateColorParam("color2", &mColor2, kMinColor, kMaxColor));
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateFloatParam( "Separation", &mSeparation )
                         .minValue(0.0f)
                         .maxValue(2.0f));
    mInterface->addParam(CreateFloatParam( "Detail", &mDetail ));
    mInterface->addParam(CreateIntParam( "Strands", &mStrands )
                         .minValue(1)
                         .maxValue(50));
    mInterface->addParam(CreateFloatParam( "Scale", &mScale )
                         .minValue(0.0f)
                         .maxValue(10.0f));
    mInterface->addParam(CreateFloatParam( "Thickness", &mThickness )
                         .minValue(0.001f));
    mInterface->addParam(CreateBoolParam( "ColorSep", &mColorSeparation ));
    mInterface->addParam(CreateBoolParam( "BackgroundFlash", &mBackgroundFlash ));
    
//    mInterface->gui()->addColumn();
//    vector<string> bgReactionNames;
//#define CIRCLEWAVE_BG_REACTION_ENTRY( nam, enm ) \
//bgReactionNames.push_back(nam);
//    CIRCLEWAVE_BG_REACTION_TUPLE
//#undef  CIRCLEWAVE_BG_REACTION_ENTRY
//    interface->addEnum(CreateEnumParam( "bg_reaction", (int*)(&mBackgroundReaction) )
//                       .maxValue(BG_REACTION_COUNT)
//                       .oscReceiver(name)
//                       .isVertical(), bgReactionNames);
    
    mApp->getAudioInputHandler().setupInterface(mInterface, "global");
}

void CircleWave::update(double dt)
{
    Scene::update(dt);
    
    //mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mElapsedTime += dt;
}

void CircleWave::draw()
{
    gl::pushMatrices();

    drawScene();
    
    gl::popMatrices();
}

void CircleWave::shaderPreDraw()
{
    // audio texture
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().bindTexture(1);
    }
    
    mShader.bind();
    
    Vec3f resolution = Vec3f( mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f );
    
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)mElapsedTime );
    mShader.uniform( "iChannel0", 1 );
    mShader.uniform( "iColor1", mColor1 );
    mShader.uniform( "iColor2", mColor2 );
    
    mShader.uniform( "iSeparation", mSeparation );
    mShader.uniform( "iDetail", mDetail );
    mShader.uniform( "iStrands", mStrands );
    mShader.uniform( "iScale", mScale );
    mShader.uniform( "iColorSep", mColorSeparation );
    mShader.uniform( "iThickness", mThickness );
    mShader.uniform( "iBackgroundFlash", mBackgroundFlash );
    //mShader.unfirom( "iBgReaction", (int)mBackgroundReaction );
}

void CircleWave::drawShaderOutput()
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

void CircleWave::shaderPostDraw()
{
    mShader.unbind();
    
    if( mApp->getAudioInputHandler().hasTexture() )
    {
        mApp->getAudioInputHandler().getFbo().unbindTexture();
    }
}

void CircleWave::drawScene()
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

void CircleWave::drawDebug()
{
    mApp->getAudioInputHandler().drawDebug(mApp->getViewportSize());
}
