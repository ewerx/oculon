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
    //void setupParams(params::InterfaceGl& params);
    void reset();
    void resize();
    void update(double dt);
    void draw();
    bool handleKeyDown(const KeyEvent& keyEvent);
    void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseUp( const ci::app::MouseEvent& event);
	void handleMouseDrag( const ci::app::MouseEvent& event );
    
private:
    void updateAudioResponse();
    
private:
    Vec2i mMousePos;
    
    float mTimeStep;
	float mLineOpacity;
    float mPointOpacity;
	float mParticleNeighborhood;
    float mParticleRepulsion;
	float mCenterAttraction;
	
	int mKParticles;
    bps::ParticleSystem mParticleSystem;
	bool mIsMousePressed;
    bool mSlowMotion;
};

#endif // __MAGNETOSPHERE_H__
