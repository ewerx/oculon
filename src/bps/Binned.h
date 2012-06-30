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
#include "OscMessage.h"

#include "Constants.h"
#include "cinder/Timeline.h"

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
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    bool handleKeyDown(const ci::app::KeyEvent& keyEvent);
    void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseUp( const ci::app::MouseEvent& event);
	void handleMouseDrag( const ci::app::MouseEvent& event );
    
    // new
    void addRepulsionForce( const Vec2f& pos, float radius, float force );
    
    void handleOscMultiTouch( const ci::osc::Message& message );
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
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
    float mPointSize;
    float mLineWidth;
    
    bool mBounceOffWalls;
    bool mCircularWall;
    float mCircularWallRadius;
    float mWallPadding;
    
    
    enum eAudioPattern
    {
        AUDIO_PATTERN_LINE,
        AUDIO_PATTERN_TOPBOTTOM,
        AUDIO_PATTERN_RANDOM,
        AUDIO_PATTERN_SEGMENTS,
        AUDIO_PATTERN_METROPOLIS1,
        AUDIO_PATTERN_METROPOLIS2,
        
        AUDIO_PATTERN_COUNT
    };
    int mAudioPattern;
    
    enum eForcePattern
    {
        PATTERN_NONE,
        PATTERN_AUDIO,
        PATTERN_FOUR_CORNERS,
        PATTERN_BPM_BOUNCE,
        PATTERN_CROSSING,
        PATTERN_RING,
        PATTERN_DISSOLVE,
        PATTERN_WAVE,
        PATTERN_METROPOLIS,
        
        PATTERN_COUNT
    };
    int mApplyForcePattern;
    
    float mBpmBounceTime;
    int mBeatCount;
    int mBpmBouncePosition;
    int mFrameCounter;
    float mPatternDuration;
    bool mShowForceIndicators;
    float mForceIndicatorScale;
    ColorAf mForceIndicatorColor;
    
    Vec2i mCrossForcePoint[4];
    
    
    bool mIsOrbiterModeEnabled;
    
    float mAudioSensitivity;
    
    enum eRepulsionMode
    {
        MODE_RADIUS,
        MODE_FORCE,
        MODE_BOTH,
        
        MODE_COUNT
    };
    int mRepulsionMode;
    
    struct tForce
    {
        float _x;
        float _y;
        float _radius;
        float _magX;
        float _magY;
    
        tForce( float x, float y, float radius, float magX, float magY )
        : _x(x), _y(y), _radius(radius), _magX(magX), _magY(magY) {}
        
    };
    
    enum eInitialFormation
    {
        FORMATION_NONE,
        FORMATION_DISC,
        FORMATION_RING,
        FORMATION_WAVE,
        FORMATION_MULTIWAVE,
        
        FORMATION_COUNT
    };
    int mInitialFormation;
    
    // deepfield
    int mNumSegments;
    float mSegmentWidth;
    
    struct tRepulsionForce
    {
        Vec2f mPos;
        float mRadius;
        float mForce;
    };
    
    std::queue<tRepulsionForce> mQueuedForces;
    
    // metropolis
    tMetropolisPattern mMetropolis;
    Anim<float> mTraverse;
    
};

#endif // __MAGNETOSPHERE_H__
