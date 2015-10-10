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
#include "DustRenderer.h"
#include "GravitonRenderer.h"
#include "FlockRenderer.h"
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

private:
    // particle system
    ParticleController mParticleController;
    
    ci::gl::GlslProg mSimulationShader;
    
    // predators
    int mPredatorBufSize;
    ParticleController mPredatorController;
    ci::gl::GlslProg mPredatorSimShader;

    // camera
    CameraController mCameraController;
    
    // params
    TimeController mTimeController;
    
    bool mReset;
    
    // audio
    AudioInputHandler mAudioInputHandler;
};


