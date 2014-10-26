/*
 *  Body.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __BODY_H__
#define __BODY_H__

#include "cinder/Color.h"
#include "cinder/Matrix.h"
#include "cinder/PolyLine.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "Entity.h"
#include "TextEntity.h"

class Scene;
class Orbiter;

//
// An orbital body
//
class Body : public Entity<double>
{
public:
    Body(Orbiter* orbiter,
         std::string name,
         const ci::Vec3d& pos,
         const ci::Vec3d& vel,
         float radius, 
         double rotationSpeed,
         double mass, 
         const ci::ColorA& color);
    
    Body(Orbiter* orbiter,
         std::string name,
         const ci::Vec3d& pos,
         const ci::Vec3d& vel,
         float radius, 
         double rotationSpeed,
         double mass, 
         const ci::ColorA& color,
         ci::ImageSourceRef textureImage);
    
    virtual ~Body();
    
    // inherited from Entity
    virtual void setup();
    virtual void update(double dt);
    virtual void draw(const ci::Matrix44d& transform, bool drawBody);
    
    // new methods
    void drawTrail();
    const ci::Vec3d& getVelocity() const { return mVelocity; }
    const double& getAcceleration() const { return mAcceleration; }
    void applyForceFromBody(Body& otherBody, double dt, double gravConst);
    
    float getRadius() const { return mRadius*mRadiusMultiplier; }
    float getBaseRadius() const { return mRadius; }
    double getMass() const { return mMass; }
    void applyFftBandValue( float fftBandValue );
    const std::string& getName() const { return mName; }
    
    void setLabelVisible( bool visible ) { mIsLabelVisible = visible; }
    
    void resetTrail();
    
protected:
    void updateLabel();
    void drawDebugVectors();
    
protected:
    Orbiter* mOrbiter;
    ci::Vec3d mVelocity;
    double mAcceleration;
    double mMass;
    float mRadius;
    
    ci::Anim<float> mRadiusMultiplier;
    
    double mRotation;
    double mRotationSpeed;
    std::string mName;
    
    ci::ColorA mColor;
    ci::gl::Texture mTexture;
    bool mHasTexture;
    
    ci::PolyLine<ci::Vec3f> mMotionTrail;
    
    TextEntity mLabel;
    //TextBox mLabel;
    bool mIsLabelVisible;
    
    float mDistToCam;
    
    //TEST
    float mEaseFactor;
    std::vector<float> mLastFftValues;
    
    static GLfloat no_mat[];
    static GLfloat mat_ambient[];
    static GLfloat mat_diffuse[];
    static GLfloat mat_specular[];
    static GLfloat mat_emission[];
    static GLfloat mat_shininess[];
    static GLfloat no_shininess[];
};


#endif // __BODY_H__
