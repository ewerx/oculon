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
#include "Entity.h"
#include "TextEntity.h"

using namespace ci;
using namespace std;

class Scene;
class Orbiter;

//
// An orbital body
//
class Body : public Entity<double>
{
public:
    Body(Orbiter* orbiter,
         string name,
         const Vec3d& pos, 
         const Vec3d& vel, 
         float radius, 
         double rotationSpeed,
         double mass, 
         const ColorA& color);
    
    Body(Orbiter* orbiter,
         string name,
         const Vec3d& pos, 
         const Vec3d& vel, 
         float radius, 
         double rotationSpeed,
         double mass, 
         const ColorA& color,
         ImageSourceRef textureImage);
    
    virtual ~Body();
    
    // inherited from Entity
    virtual void setup();
    virtual void update(double dt);
    virtual void draw(const Matrix44d& transform, bool drawBody);
    
    // new methods
    void drawTrail();
    const Vec3d& getVelocity() const { return mVelocity; }
    const double& getAcceleration() const { return mAcceleration; }
    void applyForceFromBody(Body& otherBody, double dt, double gravConst);
    
    float getRadius() const { return mRadius*mRadiusMultiplier; }
    float getBaseRadius() const { return mRadius; }
    double getMass() const { return mMass; }
    void applyFftBandValue( float fftBandValue );
    const string& getName() const { return mName; }
    
    void setLabelVisible( bool visible ) { mIsLabelVisible = visible; }
    
    void resetTrail();
    
protected:
    void updateLabel();
    void drawDebugVectors();
    
protected:
    Orbiter* mOrbiter;
    Vec3d mVelocity;
    double mAcceleration;
    double mMass;
    float mRadius;
    float mPeakRadiusMultiplier;
    float mRadiusMultiplier;
    double mRotation;
    double mRotationSpeed;
    string mName;
    
    ColorA mColor;
    gl::Texture mTexture;
    bool mHasTexture;
    
    PolyLine<Vec3f> mMotionTrail;
    
    TextEntity mLabel;
    //TextBox mLabel;
    bool mIsLabelVisible;
    
    //TEST
    float mRadiusAnimRate;
    float mRadiusAnimTime;
    float mEaseFactor;
    vector<float> mLastFftValues;
    
    static GLfloat no_mat[];
    static GLfloat mat_ambient[];
    static GLfloat mat_diffuse[];
    static GLfloat mat_specular[];
    static GLfloat mat_emission[];
    static GLfloat mat_shininess[];
    static GLfloat no_shininess[];
};


#endif // __BODY_H__
