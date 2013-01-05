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

class Interface;

class SplineCam
{
public:
    SplineCam();
    ~SplineCam();

    void setup();
    bool resetSpline();
    void setupInterface(Interface* interface, const std::string& name);
    void update(double dt);

    void drawSpline();

    const ci::CameraPersp& getCamera()          { return mCam; }
    
    void setRadius( const float radius)         { mRadius = radius; }
    void setTarget( const ci::Vec3f& target )   { mTarget = target; }

private:
    
    ci::CameraPersp     mCam;
    ci::BSpline3f       mSpline;
    float               mSplineValue;
    float               mRadius;
    ci::Vec3f           mLastPos;
    ci::Vec3f           mTarget;
    float               mSpeed;
};

#endif /* defined(__Oculon__SplineCam__) */
