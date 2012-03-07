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
#include <vector>

#include "MSAOpenCL.h"

#include "Scene.h"
#include "ParticleController.h"
#include "MidiMap.h"

using namespace ci;
using std::vector;

#define USE_OPENGL_CONTEXT

#define kMagnetoNumParticles        (512*512)
#define kMagnetoMaxNodes            8
#define kMagnetoMaxTrailLength      2
#define kMagnetoOldNodeHistory      200

#define kMagnetoSizeofPosBuffer	(kMagnetoNumParticles * sizeof(float2) * kMagnetoMaxTrailLength)
#define kMagnetoSizeofColBuffer	(kMagnetoNumParticles * sizeof(float4) * kMagnetoMaxTrailLength)


class Magnetosphere : public Scene
{
public:
    static int  sOldNodeHistory;
    
private:
    
    typedef struct
    {
        float2 vel;
        float mass;
        float life;    // make sure the float2 vel is aligned to a 16 byte boundary
    } clParticle;
    
    typedef struct 
    {
        float2 pos;
        float spread;
        float attractForce;
        float waveAmp;
        float waveFreq;
    } clNode;
    
    enum eArgs
    {
        ARG_PARTICLES,
        ARG_POS_BUFFER,
        ARG_COLOR_BUFFER,
        ARG_NODES,
        ARG_NUM_NODES,
        ARG_COLOR,
        ARG_COLOR_TAPER,
        ARG_MOMENTUM,
        ARG_DIESPEED,
        ARG_TIME,
        ARG_WAVE_POS_MULT,
        ARG_WAVE_VEL_MULT,
        ARG_MASS_MIN
    };
    
    class OldNode
    {
    public:
        Vec2f history[kMagnetoOldNodeHistory];
        int curHead;
        int id;
        
        OldNode()
        {
            curHead = 0;
            id = 0;
        }
        
        
        void add(float px, float py) 
        {
            curHead--;
            if(curHead<0) curHead += Magnetosphere::sOldNodeHistory;
            history[curHead].set(px, py);
        }
        
        void init(float px, float py) 
        {
            curHead = 0;
            for(int i=0; i<Magnetosphere::sOldNodeHistory; i++) 
            {
                add(px, py);
            }
        }
        
        
        void draw(const float trailAudioDistortMin, const float trailAudioDistortMax, const float trailWaveFreqMin, const float trailWaveFreqMax, const bool trailDistanceAffectsWave, const float trailBrightness, const float4& color, const float2& renderDimensions) 
        {
            		glBegin(GL_LINE_STRIP);
            //		ofBeginShape();
            
            float value = Rand::randFloat(); //TODO: audio
            // value = fft.getPeaks()[(id+1) % fft.getNumAverages()]
            float distAmount = lerp<float>(trailAudioDistortMin, trailAudioDistortMax, value);
            
            value = Rand::randFloat();
            //value = fft.getPeaks()[(id+3) % fft.getNumAverages()];
            float trailWaveFreq = 100.0f * lerp<float>(trailWaveFreqMin, trailWaveFreqMax, value);
            
            float invMult = 1.0f/Magnetosphere::sOldNodeHistory;
            for(int i=0; i<Magnetosphere::sOldNodeHistory; i++) 
            {
                int index = (curHead + i) % Magnetosphere::sOldNodeHistory;
                float distRatioFromhead = i * invMult;
                
                Vec2f &cur = history[index];
                Vec2f old = Vec2f::zero();
                Vec2f old2 = Vec2f::zero();
                Vec2f next = Vec2f::zero();
                
                if(i>1) 
                {
                    old = history[(index + Magnetosphere::sOldNodeHistory - 1) % Magnetosphere::sOldNodeHistory];
                    
                    Vec2f diff = cur - old;
                    
                    Vec2f perp = Vec2f(-diff.y, diff.x);
                    perp.normalize();
                    
                    Vec2f disp = perp * distAmount * sin(trailWaveFreq * index * invMult);
                    
                    if(trailDistanceAffectsWave) 
                    {
                        disp *= 2.0f * (history[curHead] - cur).length() / renderDimensions.y;
                    }
                    cur += disp;
                } 
                else 
                {
                    old = cur;
                }
                
                if(i>2) 
                {
                    old2 = history[(index + Magnetosphere::sOldNodeHistory - 2) % Magnetosphere::sOldNodeHistory];
                } 
                else 
                {
                    old2 = old;
                }
                
                if(i<Magnetosphere::sOldNodeHistory-1) 
                {
                    next = history[(index + 1) % Magnetosphere::sOldNodeHistory];
                } 
                else 
                {
                    next = cur;
                }
                
                //			ofCurveVertex(cur.x, cur.y);
                float a = trailBrightness * (1.0f - distRatioFromhead);
                //vector<Vec2f> points;
                //points.push_back(old2);
                //points.push_back(old);
                //points.push_back(cur);
                //points.push_back(next);
                glColor4f(color.x * a, color.y * a, color.z * a, 1.0f);
                //gl::draw( Path2d( BSpline2f( points, 1, /*loop*/false, /*open*/true ) ) );
                			glVertex3f(cur.x, cur.y, 0);
                console() << "vertex: " << cur.x << ", " << cur.y << std::endl;
                //ofCurve(old2.x, old2.y, old.x, old.y, cur.x, cur.y, next.x, next.y);
                
            }
            		glEnd();
            //		ofEndShape(false);
        }
    };


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
    //void generateParticles();
    
    void initOpenCl();
    void updateParams();
    
    void updateNodes();
    void updateNode(clNode &n, int i);
    
    void preRender();
    void drawNodes();
    void drawParticles();
    void drawStrokes();
    
    void drawOffscreen();
     
private:
    
    bool                mIsMousePressed;
    float2				mMousePos;
    float2				mDimensions;
    
    clNode              mNodes[kMagnetoMaxNodes];
    MSA::OpenCLBuffer	mClBufNodes;
    OldNode             mOldNodes[kMagnetoMaxNodes];
    cl_int              mNumNodes;
    
    bool                mNodesNeedUpdating;
    
    float4              mColor;
    float2              mRenderDimensions;
    
    MSA::OpenCL			mOpenCl;
    MSA::OpenCLProgram  *mClProgram;
    MSA::OpenCLKernel	*mKernelUpdate;
    
    clParticle			mParticles[kMagnetoNumParticles];
    MSA::OpenCLBuffer	mClBufParticles;		// stores above data
    
    // contains info for rendering (VBO data) first pos, then oldPos
    float2              mPosBuffer[kMagnetoNumParticles * kMagnetoMaxTrailLength];
    MSA::OpenCLBuffer	mClBufPosBuffer;
    
    // contains info for rendering (VBO data)
    float4				mColorBuffer[kMagnetoNumParticles * kMagnetoMaxTrailLength];
    MSA::OpenCLBuffer	mClBufColorBuffer;
    
    GLuint              mIndices[kMagnetoNumParticles * kMagnetoMaxTrailLength];
    
    gl::Texture         mParticleTexture;
    GLuint				mVbo[2];

    float               mSpreadMin;
    float               mSpreadMax;
    float               mNodeAttractMin;
    float               mNodeAttractMax;
    float				mWaveFreqMin;
    float				mWaveFreqMax;
    float				mWaveAmpMin;
    float				mWaveAmpMax;
    float				mCenterDeviation;
    float				mCenterDeviationMin;
    float				mCenterDeviationMax;
    
    cl_float			mColorTaper;
    cl_float			mMomentum;
    cl_float			mDieSpeed;
    cl_float			mAnimTime;
    cl_float			mTimeSpeed;
    cl_float			mWavePosMult;
    cl_float			mWaveVelMult;
    cl_float			mMassMin;
    int					mPointSize;
    int					mLineWidth;
    cl_int				mNumParticles;
    int					mNumParticlesPower;
    float				mFadeSpeed;
    float				mNodeRadius;
    float				mNodeBrightness;
    
    // rendering / fx
    bool				mEnableBlending;
    bool				mAdditiveBlending;
    bool				mDoDrawLines;
    bool				mDoDrawPoints;
    bool				mDoDrawNodes;
    bool				mEnableLineSmoothing;
    bool				mEnablePointSmoothing;
    bool                mUseImageForPoints;
    
    gl::Fbo             mFboNew;
    gl::Fbo             mFboComp;
    gl::Fbo             mFboBlur;
    gl::GlslProg        mBlurShader;
    
    bool                mUseFbo;
    bool                mDoBlur;
    int                 mBlurAmount;
    float               mTrailBrightness;
    float               mTrailAudioDistortMin;
    float               mTrailAudioDistortMax;
    float               mTrailWaveFreqMin;
    float               mTrailWaveFreqMax;
    bool                mTrailDistanceAffectsWave;
    //int				mBlurIterations		= 1;
    //float             mBlurAlpha			= 0;
    //float             mBlurCenterWeight	= 10;
    //float             mOrigAlpha			= 1;
    int                 mFboScaleDown;
    
};

#endif // __MAGNETOSPHERE_H__
