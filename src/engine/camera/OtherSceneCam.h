//
//  OtherSceneCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#pragma once

#include "SceneCam.h"

class OculonApp;

class OtherSceneCam : public SceneCam
{
public:
    OtherSceneCam(OculonApp* app, const std::string& sceneName);
    virtual ~OtherSceneCam() {}
    
    const ci::Camera& getCamera();
    
private:
    OculonApp* mApp;
};
