//
// Trails.h
// Oculon
//
// Created by Ehsan on 12-12-28.
// Copyright 2011 ewerx. All rights reserved.
//


#pragma once


#include "Scene.h"
#include "SplineCam.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include <deque>


//
// Trails
//
class Trails : public Scene
{
public:
    Trails();
    virtual ~Trails();
    
    // inherited from Scene
    void setup();
    //void reset();
    //void resize();
    void update(double dt);
    void draw();
    const ci::Camera& getCamera();
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void updateAudioResponse();
    
private:
    ci::gl::Texture			mTexture;
    
    //ci::gl::GlslProg		mShader;
    
    ci::gl::VboMesh			mVboMesh;
    
    std::deque< ci::Vec3f >	mTrail;
    
    double				mTime;
    float				mAngle;
    
    // params
    float               mTrailsPerSecond;
    float               mPhiScale;
    float               mPhiOffset;
    float               mThetaScale;
    //float               mThetaOffset;
    float               mRadius;
    float               mTwist;
    float               mWidth;
    float               mAngleIncrement;
    bool                mWireframe;
    
    enum eCamType
    {
        CAM_MANUAL,
        CAM_ORBITER,
        CAM_CATALOG,
        CAM_SPLINE,
        
        CAM_COUNT
    };
    eCamType            mCamType;
    
    SplineCam           mSplineCam;

};

