//
//  SpinCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#pragma once

#include "SceneCam.h"
#include "cinder/Timeline.h"

class SpinCam : public SceneCam
{
public:
    SpinCam(const float aspectRatio);
    virtual ~SpinCam();
    
    // from SceneCam
    void setupInterface(Interface* interface, const std::string& name);
    void update(double dt);
    
    bool onSpinDistanceChanged();
    
private:
    ci::Quatf                   mSpinQuat;
    ci::Vec3f                   mSpinRate;
    ci::Vec3f                   mTargetSpinRate;
    ci::Vec3f                   mSpinUp;
    ci::Anim<float>             mSpinDistance;
    float                       mTargetSpinDistance;
    float                       mSpinTheta;
    float                       mTransitionTime;

};
