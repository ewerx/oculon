//
//  EffectShaders.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-15.
//
//

#include "EffectShaders.h"

#include "OculonApp.h"
#include "Utils.h"

using namespace ci;
using namespace std;

EffectShaders::EffectShaders()
: Scene("effects")
, mCurrentEffect(0)
, mCurrentInputTexture(0)
{
}

EffectShaders::~EffectShaders()
{
}

void EffectShaders::setup()
{
    Scene::setup();
    
    mAudioInputHandler.setup(true);
    
    // effects
    mEffects.push_back( new FragShader("interference", "crtinterference_frag.glsl") );
    
    // inputs
    vector<Scene*> scenes;
    scenes.push_back( mApp->getScene("lines") );
    scenes.push_back( mApp->getScene("rings") );
    scenes.push_back( mApp->getScene("textureshaders") );
    scenes.push_back( mApp->getScene("audio") );
    scenes.push_back( mApp->getScene("objectshaders") );
    scenes.push_back( mApp->getScene("tilings") );
    scenes.push_back( mApp->getScene("graviton") );
    scenes.push_back( mApp->getScene("shadertest") );
    scenes.push_back( mApp->getScene("circlewave") );
    
    for (Scene* scene : scenes )
    {
        if (scene)
        {
            mInputTextures.push_back( make_pair( scene->getName(), scene->getFboTexture() ) );
        }
    }
    
    // TODO: add syphon client
    
    // noise
    mNoiseTexture.setup(256, 256);
    
    reset();
}

void EffectShaders::reset()
{
    mTimeController.reset();
}

void EffectShaders::setupInterface()
{
    mTimeController.setupInterface(mInterface, mName);
    
    // effects
    vector<string> effectNames;
    for( FragShader* effect : mEffects )
    {
        if (effect)
        {
            effectNames.push_back(effect->getName());
        }
    }
    mInterface->addEnum(CreateEnumParam( "effect", (int*)(&mCurrentEffect) )
                        .maxValue(effectNames.size())
                        .oscReceiver(getName())
                        .isVertical(), effectNames);
    
    // inputs
    vector<string> inputNames;
    for( tNamedTexture input : mInputTextures )
    {
        inputNames.push_back(input.first);
    }
    mInterface->addEnum(CreateEnumParam( "input", (int*)(&mCurrentInputTexture) )
                        .maxValue(inputNames.size())
                        .oscReceiver(getName())
                        .isVertical(), inputNames);
    
    mNoiseTexture.setupInterface(mInterface, mName);
    
    for( FragShader* effect : mEffects )
    {
        if (effect)
        {
            effect->setupInterface(mInterface, mName);
        }
    }
    
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void EffectShaders::update(double dt)
{
    Scene::update(dt);
    
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    mNoiseTexture.update(dt);
    
    // update effect
}

void EffectShaders::draw()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    gl::pushMatrices();
    
    // pre-draw
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    mInputTextures[mCurrentInputTexture].second.bind(0);
    mNoiseTexture.bindTexture(1);
    
    gl::GlslProg shader = mEffects[mCurrentEffect]->getShader();
    shader.bind();
    
    Vec2f resolution = Vec2f( mApp->getViewportWidth(), mApp->getViewportHeight() );
    
    shader.uniform( "iResolution", resolution );
    shader.uniform( "iGlobalTime", (float)mTimeController.getElapsedSeconds() );
    shader.uniform( "iChannel0", 0 );
    shader.uniform( "iChannel1", 1 );
    
    mEffects[mCurrentEffect]->setCustomParams( mAudioInputHandler );
    
    // draw
    Utils::drawTexturedRect( mApp->getViewportBounds() );

    // post-draw
    shader.unbind();
    
    mNoiseTexture.unbindTexture();
    mInputTextures[mCurrentInputTexture].second.unbind();
    
    gl::popMatrices();
    glPopAttrib();
}

#pragma mark  - Effects

TelevisionEffect::TelevisionEffect()
: FragShader("television", "crtinterference_frag.glsl")
{
    
}

void TelevisionEffect::setupInterface(Interface *interface, const std::string &prefix)
{
    string oscName = prefix + "/" + mName;
    
    
}

void TelevisionEffect::setCustomParams(AudioInputHandler &audioInputHandler)
{
    
}
