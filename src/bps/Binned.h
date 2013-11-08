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
//#include "BlobTracker.h"

#include "Constants.h"
#include "cinder/Timeline.h"

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
    void addRepulsionForce( const ci::Vec2f& pos, float radius, float force );
    
    void handleOscMultiTouch( const ci::osc::Message& message );
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
private:
    void updateAudioResponse();
    void applyQueuedForces();
    void applyForcePatterns();
    
private:
    ci::Vec2i mMousePos;
    bool mHandleMouseInput;
    
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
    
    ci::ColorAf mPointColor;
    ci::ColorAf mForceColor;
    float mPointSize;
    float mLineWidth;
    
    bool mBounceOffWalls;
    bool mCircularWall;
    float mCircularWallRadius;
    float mWallPadding;
    
    // audio pattern
#define BINNED_AUDIO_PATTERN_TUPLE \
BINNED_AUDIO_PATTERN_ENTRY( "Line", AUDIO_PATTERN_LINE ) \
BINNED_AUDIO_PATTERN_ENTRY( "TopBottom", AUDIO_PATTERN_TOPBOTTOM ) \
BINNED_AUDIO_PATTERN_ENTRY( "Random", AUDIO_PATTERN_RANDOM ) \
BINNED_AUDIO_PATTERN_ENTRY( "Segments", AUDIO_PATTERN_SEGMENTS ) \
BINNED_AUDIO_PATTERN_ENTRY( "Metro1", AUDIO_PATTERN_METROPOLIS1 ) \
BINNED_AUDIO_PATTERN_ENTRY( "Metro2", AUDIO_PATTERN_METROPOLIS2 ) \
//end tuple
    enum eAudioPattern
    {
#define BINNED_AUDIO_PATTERN_ENTRY( nam, enm ) \
enm,
        BINNED_AUDIO_PATTERN_TUPLE
#undef  BINNED_AUDIO_PATTERN_ENTRY
        
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
    ci::ColorAf mForceIndicatorColor;
    
    ci::Vec2i mCrossForcePoint[4];
    
    
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
    
    // initial formation
#define BINNED_FORMATION_TUPLE \
BINNED_FORMATION_ENTRY( "Manual", FORMATION_NONE ) \
BINNED_FORMATION_ENTRY( "Orbiter", FORMATION_DISC ) \
BINNED_FORMATION_ENTRY( "Graviton", FORMATION_RING ) \
BINNED_FORMATION_ENTRY( "Catalog", FORMATION_WAVE ) \
BINNED_FORMATION_ENTRY( "Spline", FORMATION_MULTIWAVE ) \
//end tuple
    
    enum eInitialFormation
    {
#define BINNED_FORMATION_ENTRY( nam, enm ) \
enm,
        BINNED_FORMATION_TUPLE
#undef  BINNED_FORMATION_ENTRY
        
        FORMATION_COUNT
    };
    int mInitialFormation;
    
    // deepfield
    int mNumSegments;
    float mSegmentWidth;
    
    struct tRepulsionForce
    {
        ci::Vec2f mPos;
        float mRadius;
        float mForce;
    };
    
    std::queue<tRepulsionForce> mQueuedForces;
    
    // metropolis
    tMetropolisPattern mMetropolis;
    ci::Anim<float> mTraverse;
    
    bool mInverseBassTime;
    
    //TEMP
//    bool                mEnableBlobTracker;
//    BlobTracker         mBlobTracker;
};

#endif // __MAGNETOSPHERE_H__
