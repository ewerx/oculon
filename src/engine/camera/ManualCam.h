//
//  ManualCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#pragma once

#include "SceneCam.h"
#include "cinder/Camera.h"

class ManualCam : public SceneCam
{
public:
    ManualCam( const ci::Camera& manualCam );
    virtual ~ManualCam() {}
    
    const ci::Camera& getCamera();
    
private:
    const ci::Camera& mManualCamRef;
};
