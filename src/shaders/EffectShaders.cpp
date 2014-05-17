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
    
    drawShaderOutput(viewportSize);
    
    mShader.unbind();
}


void EffectShaders::Effect::drawShaderOutput(const ci::Vec2i& viewportSize)
{
    // Draw shader output
    gl::enable( GL_TEXTURE_2D );
    gl::color( Colorf::white() );
    gl::begin( GL_TRIANGLES );
    
    // Define quad vertices
    const Area& bounds = Area( 0, 0, viewportSize.x, viewportSize.y );
    
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
