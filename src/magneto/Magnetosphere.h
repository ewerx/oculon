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
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Timeline.h"
#include <vector>

#include "MSAOpenCL.h"

#include "Scene.h"
#include "MotionBlurRenderer.h"


class Magnetosphere : public Scene
{
private:
    
    typedef struct
    {
        float2 mVel;
        cl_float mMass;
        cl_float mLife;    
        // make sure the float2 vel is aligned to a 16 byte boundary
    } tParticle;
    
    typedef struct 
    {
        float2 mPos;
        float2 mVel;
        cl_float mMass;
        cl_float mCharge;
    } tNode;
    
    enum eArgs
    {
        ARG_PARTICLES,
        ARG_NODES,
        ARG_POS,
        ARG_COLOR,
        ARG_FFT,
        ARG_NUM_PARTICLES,
        ARG_NUM_NODES,
        ARG_DT,
        ARG_DAMPING,
        ARG_ALPHA,
        ARG_FLAGS,
        ARG_MOUSEPOS,
        ARG_DIMENSIONS
    };
    
    enum
    {
        kStep =                 1024,
        kMaxParticles =         (64 * kStep),
        kFftBands =             512,
        kMaxNodes =             4,
        kParticleTrailSize =    4,
    };
    
    // duplicates definition in CL kernel
    enum eParticleFlags
    {
        PARTICLE_FLAGS_NONE =       0x00,
        PARTICLE_FLAGS_INVSQR =     (1 << 0),
        PARTICLE_FLAGS_SHOW_DARK =  (1 << 1),
        PARTICLE_FLAGS_SHOW_SPEED = (1 << 2),
        PARTICLE_FLAGS_SHOW_MASS =  (1 << 3),
    };
    
public:
    Magnetosphere();
    virtual ~Magnetosphere();
    
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

protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
private:
    void updateAudioResponse();
    //void generateParticles();
    
    void initOpenCl();
    void initParticles();
    
    void updateNodes(double dt);
    //void updateNode(clNode &n, int i);
    
    void preRender();
    //void drawNodes();
    void drawParticles();
    //void drawStrokes();
    
private:
    
    bool                mIsMousePressed;
    float2				mMousePos;
    float2              mDimensions;
    
    // opencl
    MSA::OpenCL			mOpenCl;
    MSA::OpenCLProgram  *mClProgram;
    MSA::OpenCLKernel	*mKernel;
    
    MSA::OpenCLBuffer	mClBufParticles;
    MSA::OpenCLBuffer   mClBufNodes;
    MSA::OpenCLBuffer	mClBufPos;
    MSA::OpenCLBuffer	mClBufColor;
    MSA::OpenCLBuffer   mClBufFft;
    
    // opencl data
    tParticle			mParticles[kMaxParticles];
    tNode               mNodes[kMaxNodes];
    float2              mPosBuffer[kMaxParticles * kParticleTrailSize];
    float4				mColorBuffer[kMaxParticles * kParticleTrailSize];
    cl_float            mAudioFft[kFftBands];
    
    // opencl params
    cl_uint             mNumParticles;
    cl_uint             mNumNodes;
    cl_float            mTimeStep;
    cl_float            mDamping;
    cl_uint             mFlags;
    
    // sim params
    bool                mUseInvSquareCalc;
    
    // rendering
    GLuint				mVbo[2]; // pos and color VBOs
    gl::Texture         mParticleTexture;
    float				mPointSize;
    float				mLineWidth;
    float               mParticleAlpha;
    bool				mEnableBlending;
    bool				mAdditiveBlending;
    bool				mEnableLineSmoothing;
    bool				mEnablePointSmoothing;
    bool                mUseImageForPoints;
    bool                mScalePointsByDistance;
    bool                mUseMotionBlur;
    
    MotionBlurRenderer  mMotionBlurRenderer;
    
    //TODO: purpose?
    //GLuint              mIndices[kMaxParticles * kMaxTrailSize];

    float               mNodeRotation;
    float               mNodeRotationSpeed;
    
};

#endif // __MAGNETOSPHERE_H__
