//
//  CameraController.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-03-14.
//
//

#pragma once

#include "Interface.h"
#include "SceneCam.h"
#include "cinder/Camera.h"
#include "cinder/Timeline.h"

class OculonApp;
class Scene;

class CameraController
{
public:
    CameraController();
    virtual ~CameraController() {}
    
    void setup(OculonApp *app);
    void setupInterface(Interface *interface, const std::string& sceneName);
    
    void update(double dt);
    
    void setCamIndex( const int index )   { mCurrentCameraIndex = index; }
    const ci::Camera& getCamera();
    
    void addCamera( SceneCam* sceneCam );
    
protected:
    const std::vector<std::string> getCameraNames();
    bool onCameraChanged();
    
    
protected:
    float                       mAspectRatio;
    
    std::vector<SceneCam*>      mCameras;
    int                         mCurrentCameraIndex;
};

