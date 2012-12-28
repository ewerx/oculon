//
// Trails.h
// Oculon
//
// Created by Ehsan on 12-12-28.
// Copyright 2011 ewerx. All rights reserved.
//


#pragma once


#include "Scene.h"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include <deque>

using namespace ci;

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
    
protected:// from Scene
    //void setupInterface();
    //void setupDebugInterface();
    
private:
    void updateAudioResponse();
    
private:
    gl::Texture			mTexture;
    
    //gl::GlslProg		mShader;
    
    gl::VboMesh			mVboMesh;
    
    std::deque< Vec3f >	mTrail;
    
    double				mTime;
    float				mAngle;

};

