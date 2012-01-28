/*
 *  Particle.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-23.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "cinder/Vector.h"
#include "cinder/Perlin.h"
#include <boost/circular_buffer.hpp>
#include "Entity.h"

using namespace ci;

class Particle : public Entity<float> 
{
public:
    enum eState
    {
        STATE_NORMAL,
        STATE_EXCITED,
        STATE_DECAY,
        STATE_DEAD,
        
        STATE_COUNT
    };
public:
    Particle();
    Particle(const Vec3f& pos, const Vec3f& vel, float radius, float mass, float charge, float lifespan);
    virtual ~Particle();
    
    // inherited from Entity
    virtual void update(double dt);
    virtual void draw(bool asSphere);
    
    void reset(const Vec3f& pos, const Vec3f& vel, float radius, float mass, float charge, float lifespan);
    
    inline const Vec3f& getVelocity() const     { return mVelocity; }
    inline void setVelocity( const Vec3f& vel ) { mVelocity = vel; }
    
    inline const Vec3f& getAccel() const        { return mAccel; }
    inline void setAccel( const Vec3f& acc )    { mAccel = acc; }
    
    inline float getCharge() const              { return mCharge; }
    
    bool isState( eState state )        { return (mState == state); }
    
    void drawTrail();
    void applyGravity(double dt);
    void applyPerlin(Perlin& perlin, int counter, double dt);
    void applyRepulsion(Particle& other, double dt);
    
private:
    void renderLineStripTrail();
    void renderQuadStripTrail();
    void reset();
    
    
public://private:
    //float   mInvLen;
    Vec3f   mVelocity;
    Vec3f   mAccel;
    
    float   mRadius;
    float   mMass;
    float   mInvMass;
    float   mCharge;
    float   mAge;
    float   mLifeSpan;
    float   mAgePer;
    
    eState mState;
    
    enum { TRAIL_LENGTH = 15 };
    boost::circular_buffer<Vec3f> mPosHistory;
    
};

#endif // __PARTICLE_H__
