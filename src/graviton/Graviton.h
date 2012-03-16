/*
 *  Graviton.h
 *  Oculon
 *
 *  Created by Ehsan on 11-11-26.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __GRAVITON_H__
#define __GRAVITON_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include <vector>

#include "MSAOpenCL.h"

#include "Scene.h"
#include "ParticleController.h"
#include "MidiMap.h"

using namespace ci;
using std::vector;


#define FREEOCL_VERSION


class Graviton : public Scene
{
public:
    Graviton();
    virtual ~Graviton();
    
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
    
    void initParticles();
    void initOpenCl();
    void updateParams();
        
    void preRender();
    void drawParticles();
     
private:
    
    enum
    {
        kStep =                 1024,
        kNumParticles =         (8 * kStep),
    };
    
#if defined (FREEOCL_VERSION)
    enum eClArgs
    {
        ARG_POS_IN,
        ARG_POS_OUT,
        ARG_VEL_IN,
        ARG_VEL_OUT,
        ARG_COLOR,
        ARG_COUNT,
        ARG_STEP,
        ARG_DT,
        ARG_DAMPING,
        ARG_FLAGS,
        //ARG_EPS
        
    };
#else
    enum eClArgs
    {
        ARG_POS_IN,
        ARG_POS_OUT,
        ARG_VEL,
        ARG_PBLOCK,
        ARG_DT,
        ARG_EPS
        
    };
#endif
    
    bool                    mIsMousePressed;
    float2                  mMousePos;
    
    MSA::OpenCL             mOpenCl;
    MSA::OpenCLProgram      *mClProgram;
    MSA::OpenCLKernel       *mKernel;
    
    MSA::OpenCLBuffer       mClBufPos0;
    MSA::OpenCLBuffer       mClBufPos1;
    MSA::OpenCLBuffer       mClBufVel0;
    MSA::OpenCLBuffer       mClBufVel1;
    MSA::OpenCLBuffer       mClBufColor;
    bool                    mSwap;
    
    MSA::OpenCLBuffer       mClBufPos;
    
    float4                  mPosAndMass[kNumParticles];
    float4                  mVel[kNumParticles];
    float4                  mColor[kNumParticles];
    
    cl_uint                 mStep;
    cl_uint                 mNumParticles;
    
    GLuint                  mVbo[2];
    
    
    cl_float                mTimeStep;
    cl_float                mDamping;
    cl_uint                 mFlags;
    
//    clNode              mNodes[kMaxNodes];
//    MSA::OpenCLBuffer	mClBufNodes;
//    cl_int              mNumNodes;
//    
//    bool                    mNodesNeedUpdating;
//    
//    float4              mColor;
//    float2              mRenderDimensions;
//    
    
//    
//    clParticle			mParticles[kNumParticles];
//    MSA::OpenCLBuffer	mClBufParticles;		// stores above data
//    
//    // contains info for rendering (VBO data) first pos, then oldPos
//    float2              mPosBuffer[kNumParticles * kMaxTrailLength];
//    MSA::OpenCLBuffer	mClBufPosBuffer;
//    
//    // contains info for rendering (VBO data)
//    float4				mColorBuffer[kNumParticles * kMaxTrailLength];
//    MSA::OpenCLBuffer	mClBufColorBuffer;
//    
//    GLuint              mIndices[kNumParticles * kMaxTrailLength];
//    
    gl::Texture         mParticleTexture;
//    GLuint				mVbo[2];
//
//    float               mSpreadMin;
//    float               mSpreadMax;
//    float               mNodeAttractMin;
//    float               mNodeAttractMax;
//    float				mWaveFreqMin;
//    float				mWaveFreqMax;
//    float				mWaveAmpMin;
//    float				mWaveAmpMax;
//    float				mCenterDeviation;
//    float				mCenterDeviationMin;
//    float				mCenterDeviationMax;
//    
//    cl_float			mColorTaper;
//    cl_float			mMomentum;
//    cl_float			mDieSpeed;
//    cl_float			mAnimTime;
//    cl_float			mTimeSpeed;
//    cl_float			mWavePosMult;
//    cl_float			mWaveVelMult;
//    cl_float			mMassMin;
//    int					mPointSize;
//    int					mLineWidth;
//    cl_int				mNumParticles;
//    int					mNumParticlesPower;
//    float				mFadeSpeed;
//    float				mNodeRadius;
//    float				mNodeBrightness;
//    
//    // rendering / fx
//    bool				mEnableBlending;
//    bool				mAdditiveBlending;
//    bool				mDoDrawLines;
//    bool				mDoDrawPoints;
//    bool				mDoDrawNodes;
//    bool				mEnableLineSmoothing;
//    bool				mEnablePointSmoothing;
    bool                mUseImageForPoints;
    bool                mUseInvSquareCalc;
//    
//    gl::Fbo             mFboNew;
//    gl::Fbo             mFboComp;
//    gl::Fbo             mFboBlur;
    gl::GlslProg        mBlurShader;
//    
//    bool                mUseFbo;
//    bool                mDoBlur;
//    int                 mBlurAmount;
//    float               mTrailBrightness;
//    float               mTrailAudioDistortMin;
//    float               mTrailAudioDistortMax;
//    float               mTrailWaveFreqMin;
//    float               mTrailWaveFreqMax;
//    bool                mTrailDistanceAffectsWave;
//    //int				mBlurIterations		= 1;
//    //float             mBlurAlpha			= 0;
//    //float             mBlurCenterWeight	= 10;
//    //float             mOrigAlpha			= 1;
//    int                 mFboScaleDown;
    
};

#endif // __Graviton_H__
