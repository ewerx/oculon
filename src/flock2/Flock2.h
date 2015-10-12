//
//  Flock2.h
//  Oculon
//
//  Created by Ehsan Rezaie on 10/10/2015.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "CameraController.h"
#include "ParticleController.h"
#include "Scene.h"
#include "SimplexNoiseTexture.h"
#include "TimeController.h"

#include "cinder/Cinder.h"
#include "cinder/gl/GlslProg.h"
#include <vector>

//
// Flock2
//
class Flock2 : public Scene
{
public:
    Flock2();
    virtual ~Flock2();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    ////void setupDebugInterface();

private:
    void setupParticles(const int bufSize);
    void updateParticles(double dt);
    
    void setupPredators(const int bufSize);
    void updatePredators(double dt);
    
    void setupLanterns(const int bufSize);
    void updateLanterns(double dt);

private:
    // prey
    ParticleController mParticleController;
    ci::gl::GlslProg mSimulationShader;
    
    // predators
    ParticleController mPredatorController;
    ci::gl::GlslProg mPredatorSimShader;
    
    // lanterns
    ParticleController mLanternController;
    ci::gl::GlslProg mLanternSimShader;

    // camera
    CameraController mCameraController;
    
    // params
    TimeController mTimeController;
    
    bool mReset;
    
    // audio
    AudioInputHandler mAudioInputHandler;
    
    ci::Vec3f mBounds;
};


