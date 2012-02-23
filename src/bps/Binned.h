/*
 *  Binned.h
 *  Oculon
 *
 *  Created by Ehsan on 12-01-30.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __BINNED_H__
#define __BINNED_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include <vector>
#include <queue>

#include "Scene.h"
#include "MidiMap.h"
#include "BinnedParticleSystem.h"

using namespace ci;
using std::vector;

//
// kBinned
//
class Binned : public Scene
{
public:
    Binned();
    virtual ~Binned();
    
    // inherited from Scene
    void setup();
    void setupParams(params::InterfaceGl& params);
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const KeyEvent& keyEvent);
    void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseUp( const ci::app::MouseEvent& event);
	void handleMouseDrag( const ci::app::MouseEvent& event );
    
    // new
    void addRepulsionForce( const Vec2f& pos, float radius, float force );
    
private:
    void updateAudioResponse();
    void applyQueuedForces();
    
private:
    Vec2i mMousePos;
    
    float mTimeStep;
	float mParticleNeighborhood;
    float mParticleRepulsion;
	float mCenterAttraction;
	
	int mKParticles;
    bps::ParticleSystem mParticleSystem;
	bool mIsMousePressed;
    bool mSlowMotion;
    
    float mForceScaleX;
    float mForceScaleY;
    
    float mMinForce;
    float mMinRadius;
    float mMaxForce;
    float mMaxRadius;
    
    float mDamping;
    float mWallDamping;
    
    ColorAf mPointColor;
    ColorAf mForceColor;
    float mParticleRadius;
    
    bool mRandomPlacement;
    bool mBounceOffWalls;
    
    enum eForcePattern
    {
        PATTERN_NONE,
        PATTERN_FOUR_CORNERS,
        PATTERN_BPM_BOUNCE,
        PATTERN_CROSSING,
        PATTERN_RING,
        PATTERN_DISSOLVE,
        
    };
    eForcePattern mApplyForcePattern;
    float mBpmBounceTime;
    int mBeatCount;
    int mBpmBouncePosition;
    int mFrameCounter;
    float mPatternDuration;
    
    Vec2i mCrossForcePoint[4];
    
    
    
    float mAudioSensitivity;
    
    enum eMode
    {
        MODE_RADIUS,
        MODE_FORCE,
        MODE_BOTH,
        
        MODE_COUNT
    };
    int mMode;
    
    bool mTopBottom;
    
    struct tRepulsionForce
    {
        Vec2f mPos;
        float mRadius;
        float mForce;
    };
    
    std::queue<tRepulsionForce> mQueuedForces;
};

#endif // __MAGNETOSPHERE_H__
