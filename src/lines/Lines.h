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
#include "TimeController.h"

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
    void updateParticles(double dt);
    
    void setupNodes(const int bufSize);
    void updateNodes(double dt);

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
MOTION_ENTRY( "Nodes", MOTION_NODES ) \
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
    
    // nodes
    int mNodeBufSize;
    ParticleController mNodeController;
    ci::gl::GlslProg mNodeSimShader;
    // node motion
#define NODE_MOTION_TUPLE \
NODE_MOTION_ENTRY( "Static", NODE_MOTION_STATIC ) \
NODE_MOTION_ENTRY( "Bounce", NODE_MOTION_BOUNCE ) \
NODE_MOTION_ENTRY( "Noise", NODE_MOTION_NOISE ) \
// end tuple
    
    enum eNodeMotion
    {
#define NODE_MOTION_ENTRY( nam, enm ) \
enm,
        NODE_MOTION_TUPLE
#undef  NODE_MOTION_ENTRY
        
        NODE_MOTION_COUNT
    };
    eNodeMotion mNodeMotion;
    std::vector<ci::Vec3f> mNodePositions;

    // camera
    CameraController mCameraController;
    
    // params
    TimeController mTimeController;
    
    bool mReset;
    
    float mContainmentRadius;
    
    // audio
    AudioInputHandler mAudioInputHandler;
    bool mAudioTime;
};


