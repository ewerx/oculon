//
//  OrbiterSystem.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#pragma once

#include "cinder/gl/Texture.h"

class Body
{
public:
    Body(const std::string& name,
         const ci::Vec3d& pos,
         const ci::Vec3d& vel,
         float radius,
         double rotationSpeed,
         double mass);
    
    virtual ~Body() {};
    
    void setTextures(ci::ImageSourceRef diffuse,
                     ci::ImageSourceRef normalMap);
    
    void applyForceFromBody(Body& otherBody, double dt, float gravitation);
    void update(double dt);
    void draw();
    
protected:
    std::string mName;
    float mRadius;  // AU
    float mMass;    // Earth-masses
    double mRotationSpeed;
    double mRotation;
    
    ci::Vec3f mPosition;
    ci::Vec3f mVelocity;
    ci::Vec3f mAcceleration;
    
    // textures
    ci::gl::Texture     mTexDiffuse;
    ci::gl::Texture     mTexNormal;
};

typedef std::shared_ptr<Body> BodyRef;
typedef std::vector<BodyRef> BodyList;

class System
{
public:
    System(const std::string& name);
    ~System();
    
    void addBody( BodyRef body );
    const BodyList& getBodies() const { return mBodies; }
    
    void update( double dt );
    void draw();
    
public:
    float mGravitation;
    
private:
    std::string mName;
    BodyList mBodies;
    
};
