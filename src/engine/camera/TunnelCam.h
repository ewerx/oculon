//
//  TunnelCam.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-03-29.
//
//

#ifndef __Oculon__TunnelCam__
#define __Oculon__TunnelCam__

#include "cinder/Vector.h"
#include "cinder/Camera.h"
#include "cinder/Timeline.h"
#include "cinder/Bspline.h"

class Interface;

class TunnelCam
{
public:
    TunnelCam();
    ~TunnelCam();
    
    void setup( const ci::Vec3f& startPos = ci::Vec3f::zero(), bool loop =false );
    bool reset();
    void setupInterface(Interface* interface, const std::string& name);
    void update(double dt);
    
    const ci::CameraPersp& getCamera()          { return mCam; }
    
    void setTarget( const ci::Vec3f& target )   { mTarget = target; }
    const ci::Vec3f& getPosition() const        { return mLastPos; }
    
    float mRadius;
    
private:
    
    ci::CameraPersp     mCam;
    ci::Vec3f           mLastPos;
    ci::Vec3f           mTarget;
    ci::Vec3f           mStartPos;
    float               mSpeed;
    float               mSpeedMultiplier;
    float               mTheta;
    bool                mLoop;
};

#endif /* defined(__Oculon__TunnelCam__) */
