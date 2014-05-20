//
//  Lines.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/21/2013.
//
//

#pragma once

#include "AudioInputHandler.h"
#include "CameraController.h"
#include "DustRenderer.h"
#include "GravitonRenderer.h"
#include "LinesRenderer.h"
#include "ParticleController.h"
#include "Scene.h"
#include "SimplexNoiseTexture.h"

#include "cinder/Cinder.h"
#include "cinder/gl/GlslProg.h"
#include <vector>

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
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    ////void setupDebugInterface();

private:
    void setupParticles(const int bufSize);

private:
    // particle system
    ParticleController mParticleController;
    
    ci::gl::GlslProg mSimulationShader;
    
    SimplexNoiseTexture mDynamicTexture;
    
    // motion
#define MOTION_TUPLE \
MOTION_ENTRY( "Static", MOTION_STATIC ) \
MOTION_ENTRY( "Noise", MOTION_NOISE ) \
MOTION_ENTRY( "Gravity", MOTION_GRAVITY ) \
MOTION_ENTRY( "AudioWave", MOTION_AUDIO_WAVE ) \
MOTION_ENTRY( "AudioFFT", MOTION_AUDIO_FFT ) \
// end tuple
    
    enum eMotion
    {
#define MOTION_ENTRY( nam, enm ) \
enm,
        MOTION_TUPLE
#undef  MOTION_ENTRY
        
        MOTION_COUNT
    };
    eMotion mMotion;

    // camera
    CameraController mCameraController;
    
    // params
    float mTimeStep;
    
    bool mReset;
    
    float mContainmentRadius;
    
    // audio
    AudioInputHandler mAudioInputHandler;
    bool mAudioTime;
};


