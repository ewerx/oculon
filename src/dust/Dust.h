//
//  Dust.h
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
#include "DustRenderer.h"
#include "LinesRenderer.h"
#include "SimplexNoiseTexture.h"
#include "ParticleController.h"

//
// Dust
//
class Dust : public Scene
{
public:
    Dust();
    virtual ~Dust();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void setupParticles(const int bufSize);
    
    SimplexNoiseTexture mDynamicTexture;
    
private:
    ParticleController mParticleController;
    
    ci::gl::GlslProg mSimulationShader;
    
    
    ci::CameraOrtho mCamera;
    
    // params
    float mTimeStep;
    float mDecayRate;
    
    bool mReset;
    
    std::vector<std::string> mMotionTypes;
    int mMotion;
    
    AudioInputHandler mAudioInputHandler;
    
    // MUTEK HACKS
    bool mAudioReactive;
    bool mAudioTime;
};


