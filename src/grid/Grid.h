//
// Grid.h
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
#include "cinder/Surface.h"
#include <deque>

//
// Grid
//
class Grid : public Scene
{
public:
    Grid();
    virtual ~Grid();
    
    // inherited from Scene
    void setup();
    void reset();
    //void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    
    //HACKS!
    ci::gl::Fbo& getVtfFbo() { return mVtfFbo; }
    
protected:// from Scene
    void setupInterface();
    void setupDebugInterface();
    
private:
    void updateAudioResponse();
    
    void setupDynamicTexture();
    void drawDynamicTexture();
    void drawFromDynamicTexture();
    void drawPixels();
    
private:
    ci::gl::Texture             mTexture;
    
    ci::Vec2f                   mZoom;
    
    // PERLIN
    ci::gl::Fbo					mVtfFbo;
	ci::gl::GlslProg			mShaderTex;
	float						mDisplacementSpeed;
    float                       mDisplacementHeight;
	float						mTheta;
    ci::gl::GlslProg			mShaderFractal;
    ci::Vec3f                   mNoiseScale;
    
    // pixelate
    ci::gl::GlslProg			mShaderPixelate;
    
    // params
    bool                        mDrawDynamicTexture;

};

