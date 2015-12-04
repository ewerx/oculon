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
#include "EnumSelector.h"

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
    
    bool onBehaviorChange();

private:
    // particle system
    ParticleController mParticleController;
    
    // particle behavior
    ShaderSelector mBehaviorSelector;
    
    SimplexNoiseTexture mDynamicTexture;
    
    // nodes
    int mNodeBufSize;
    ParticleController mNodeController;
    
    // node behavior
    ShaderSelector mNodeBehaviorSelector;
    std::vector<ci::Vec3f> mNodePositions;

    // camera
    CameraController mCameraController;
    
    // params
    TimeController mTimeController;
    
    bool mReset;
    
    float mContainmentRadius;
    
    float               mHarmonicX;
    float               mHarmonicY;
    float               mHarmonicRate;
    
    // audio
    AudioInputHandler mAudioInputHandler;
    bool mAudioTime;
    
    bool mFlockNodes;
};


