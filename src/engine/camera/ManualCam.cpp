//
//  ManualCam.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#include "ManualCam.h"

using namespace ci;

ManualCam::ManualCam(const Camera& manualCam)
: SceneCam("manual", 0.0f)
, mManualCamRef(manualCam)
{
}

const ci::Camera& ManualCam::getCamera()
{
    return mManualCamRef;
}
