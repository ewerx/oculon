//
//  EffectShaders.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-15.
//
//

#pragma once

#include "Scene.h"
#include "TimeController.h"
#include "AudioInputHandler.h"
#include "SimplexNoiseTexture.h"
#include "FragShader.h"

#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class EffectShaders : public Scene
{
public:
    EffectShaders();
    virtual ~EffectShaders();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    
protected:// from Scene
    void setupInterface();
    
private:
    void shaderPreDraw();
    void shaderPostDraw();
    
private:
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    
    // noise
    SimplexNoiseTexture mNoiseTexture;
    
    // effects
    std::vector<FragShader*> mEffects;
    int mCurrentEffect;
    
    // inputs
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mInputTextures;
    int mCurrentInputTexture;
};

#pragma mark - Effects

class TelevisionEffect : public FragShader
{
public:
    TelevisionEffect();
    void setupInterface( Interface* interface, const std::string& prefix );
    void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    int                 mIterations;
};


