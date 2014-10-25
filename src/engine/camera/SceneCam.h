//
//  SceneCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#pragma once

#include "cinder/Camera.h"
#include "Interface.h"

class Interface;

class SceneCam
{
public:
    SceneCam(const std::string &name, const float aspectRatio);
    ~SceneCam();
    
    virtual void setupInterface(Interface* interface, const std::string& name) { mInterfacePanel = NULL; };
    virtual void update(double dt) {};
    
    virtual const ci::Camera& getCamera()   { return mCam; }
    const std::string& getName() const      { return mName; }
    void showInterfacePanel(bool show);
    
protected:
    ci::CameraPersp             mCam;
    std::string                 mName;
    mowa::sgui::PanelControl*   mInterfacePanel;
    
};