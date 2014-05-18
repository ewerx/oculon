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
    mEffects.push_back( new Effect("interference", "crtinterference_frag.glsl") );
    
    // inputs
    vector<Scene*> scenes;
    scenes.push_back( mApp->getScene("lines") );
    scenes.push_back( mApp->getScene("rings") );
    scenes.push_back( mApp->getScene("textureshaders") );
    scenes.push_back( mApp->getScene("audio") );
    scenes.push_back( mApp->getScene("rootfract") );
    scenes.push_back( mApp->getScene("tilings") );
    scenes.push_back( mApp->getScene("graviton") );
    
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
    for( Effect* effect : mEffects )
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
    
    gl::enableAlphaBlending();
    
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    mInputTextures[mCurrentInputTexture].second.bind(0);
    mNoiseTexture.bindTexture(1);
    
    mEffects[mCurrentEffect]->draw( mApp->getViewportSize(), mInputTextures[mCurrentInputTexture].second, mTimeController, mAudioInputHandler );
    
    mNoiseTexture.unbindTexture();
    mInputTextures[mCurrentInputTexture].second.unbind();
    
    gl::popMatrices();
    glPopAttrib();
}

#pragma mark  - Effects

EffectShaders::Effect::Effect(const std::string& name, const std::string& fragShader)
: mName(name)
{
    mShader = Utils::loadFragShader( fragShader );
}

void EffectShaders::Effect::setupInterface( Interface* interface, const std::string& name )
{
}

//void EffectShaders::Effect::update(const ci::Vec2i& viewportSize, )
//{
//}

void EffectShaders::Effect::draw(const ci::Vec2i& viewportSize, ci::gl::Texture& inputTexture, TimeController& timeController, AudioInputHandler& audioInputHandler)
{
    Vec3f resolution = Vec3f( viewportSize.x, viewportSize.y, 0.0f );
    
    mShader.bind();
    mShader.uniform( "iResolution", resolution );
    mShader.uniform( "iGlobalTime", (float)timeController.getElapsedSeconds() );
    mShader.uniform( "iChannel0", 0 );
    mShader.uniform( "iChannel1", 1 );
    
    Utils::drawTexturedRect( Area( 0, 0, viewportSize.x, viewportSize.y ) );
    
    mShader.unbind();
}
