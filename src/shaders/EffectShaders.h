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
    
protected:
    class Effect
    {
    public:
        Effect(const std::string& name, const std::string& fragShader);
        ~Effect() {}
        
//        virtual void update(const ci::Vec2i& viewportSize, TimeController& timeController, AudioInputHandler& audioInputHandler);
        void draw(const ci::Vec2i& viewportSize, ci::gl::Texture& inputTexture, TimeController& timeController, AudioInputHandler& audioInputHandler);
        virtual void setupInterface( Interface* interface, const std::string& name );
        
        const std::string& getName() { return mName; }
        
    protected:
        std::string mName;
        ci::gl::GlslProg mShader;
    };
    
private:
    // control
    TimeController      mTimeController;
    AudioInputHandler   mAudioInputHandler;
    
    // noise
    SimplexNoiseTexture mNoiseTexture;
    
    // effects
    std::vector<Effect*> mEffects;
    int mCurrentEffect;
    
    // inputs
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mInputTextures;
    int mCurrentInputTexture;
};
