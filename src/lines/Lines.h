//
//  Lines.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "PingPongFbo.h"
#include "AudioInputHandler.h"

//
// Lines
//
class Lines : public Scene
{
public:
    Lines();
    virtual ~Lines();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void setupFBO();
    void setupVBO();
    
    void initParticles();
    void updateParticles();
    void drawParticles();
    
    void setupDynamicTexture();
    void generateDynamicTexture();
    ci::gl::Fbo mDynamicTexFbo;
    ci::gl::GlslProg mDynamicTexShader;
    float mNoiseSpeed;
	float mNoiseTheta;
    ci::Vec3f mNoiseScale;
    
private:
    // particle system
    enum
    {
        kBufSize = 256,
        kNumParticles = (kBufSize * kBufSize)
    };
    
    PingPongFbo mParticlesFbo;
    ci::gl::VboMesh mVboMesh;
    ci::gl::GlslProg mSimulationShader;
    ci::gl::GlslProg mRenderShader;
    
    ci::gl::Texture mInitialPosTex;
	ci::gl::Texture mInitialVelTex;
	ci::gl::Texture mParticleDataTex;
    ci::gl::Texture mNoiseTex;
    
    ci::gl::Texture mColorMapTex;
    ci::ColorAf mColor;
    
    // params
    float mTimeStep;
    float mLineWidth;
    float mDecayRate;
    
    bool mUseDynamicTex;
    bool mReset;
    
    AudioInputHandler mAudioInputHandler;
    
    // MUTEK HACKS
    bool mAudioReactive;
    bool mAudioTime;
};


