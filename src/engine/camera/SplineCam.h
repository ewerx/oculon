//
//  SplineCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-30.
//
//

#ifndef __Oculon__SplineCam__
#define __Oculon__SplineCam__

#include "cinder/Vector.h"
#include "cinder/Camera.h"
#include "cinder/Timeline.h"
#include "cinder/Bspline.h"
#include "SceneCam.h"

class Interface;

class SplineCam : public SceneCam
{
public:
    SplineCam(const float viewportAspectRatio);
    ~SplineCam();

    void setup(const float maxDistance = 1000.0f, const float radius = 100.0f, const float aspectRatio = (16.f/9.f));
    bool resetSpline();
    
    // from SceneCam
    void setupInterface(Interface* interface, const std::string& name);
    void update(double dt);

    void drawSpline();

    const ci::CameraPersp& getCamera()          { return mCam; }
    const ci::BSpline3f& getSpline()            { return mSpline; }
    
    void setRadius( const float radius)         { mRadius = radius; }
    void setTarget( const ci::Vec3f& target )   { mTarget = target; mLookForward = false; }
    void setLookForward( bool lookForward )     { mLookForward = lookForward; }

private:
    ci::BSpline3f       mSpline;
    float               mSplineValue;
    float               mRadius;
    float               mMaxDistance;
    ci::Vec3f           mLastPos;
    ci::Vec3f           mTarget;
    float               mSpeed;
    float               mSpeedMulti;
    bool                mLookForward;
    ci::Quatf           mRotationQuat;
};

#endif /* defined(__Oculon__SplineCam__) */
