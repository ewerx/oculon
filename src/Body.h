/*
 *  Body.h
 *  OculonProto
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __BODY_H__
#define __BODY_H__

#include "cinder/Color.h"
#include "cinder/Matrix44.h"
#include "cinder/PolyLine.h"
#include "Entity.h"
#include "TextEntity.h"

using namespace ci;

//
// An orbital body
//
class Body : public Entity<double>
{
public:
    Body(const Vec3d& pos, 
         const Vec3d& vel, 
         float radius, 
         double mass, 
         const ColorA& color);
    
    virtual ~Body();
    
    // inherited from Entity
    void update(double dt);
    void draw(const Matrix44d& transform);
    
    // new methods
    const Vec3d& getVelocity() const { return mVelocity; }
    void applyForceFromBody(Body& otherBody, double dt, double gravConst);
    
    void setRadiusMultiplier( float mult ) { mRadiusMultiplier = mult; }
    
private:
    Vec3d mVelocity;
    double mMass;
    float mRadius;
    float mRadiusMultiplier;
    
    ColorA mColor;
    
    PolyLine<Vec3f> mMotionTrail;
    
    TextEntity mLabel;
};


#endif // __BODY_H__
