//
//  Orbiter.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#pragma once

#include "Scene.h"
#include "CameraController.h"
#include "TimeController.h"
#include "AudioInputHandler.h"

class Orbiter : public Scene
{
    #include "OrbiterSystem.h"
    
public:
    Orbiter();
    virtual ~Orbiter();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    
private:
    CameraController mCameraController;
    TimeController mTimeController;
    AudioInputHandler mAudioInputHandler;
    
    typedef std::shared_ptr<System> SystemRef;
    std::vector<SystemRef> mSystems;
    int mCurrentSystemIndex;
    
    
    ci::gl::GlslProg    mBodyShader;
    
    // lighting consts
    static GLfloat no_mat[];
    static GLfloat mat_ambient[];
    static GLfloat mat_diffuse[];
    static GLfloat mat_specular[];
    static GLfloat mat_emission[];
    static GLfloat mat_shininess[];
    static GLfloat no_shininess[];
};