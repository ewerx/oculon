/*
 *  Magnetosphere.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MAGNETOSPHERE_H__
#define __MAGNETOSPHERE_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include <vector>

#include "MSAOpenCL.h"

#include "Scene.h"
#include "ParticleController.h"
#include "MidiMap.h"

using namespace ci;
using std::vector;

#define USE_OPENGL_CONTEXT
#define NUM_PARTICLES (1000*1000)

//
// The Magnificent Magnetosphere!
//
class Magnetosphere : public Scene
{
public:
    Magnetosphere();
    virtual ~Magnetosphere();
    
    // inherited from Scene
    void setup();
    void setupParams(params::InterfaceGl& params);
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
    void generateParticles();
    
private:
    
    typedef struct
    {
        float2 vel;
        float mass;
        float dummy;    // need this to make sure the float2 vel is aligned to a 16 byte boundary
    } clParticle;
    
    bool                mIsMousePressed;
    float2				mMousePos;
    float2				mDimensions;
    
    MSA::OpenCL			mOpencl;
    MSA::OpenCLKernel	*mKernelUpdate;
    
    
    clParticle			mParticles[NUM_PARTICLES];
    MSA::OpenCLBuffer	mClMemParticles;		// stores above data
    
    
    float2				mParticlesPos[NUM_PARTICLES];
    MSA::OpenCLBuffer	mClMemPosVBO;		// stores above data
    
    GLuint				mVbo[1];

    
    //ParticleController mParticleController;
};

#endif // __MAGNETOSPHERE_H__
