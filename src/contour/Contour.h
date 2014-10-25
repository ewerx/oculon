//
// Contour.h
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
#include "cinder/Surface.h"
#include <deque>

#include "contours.h"


// implementation of the CRaster class for contour lines creation
class ValueMap : public CRaster
{
    double *values;
public:
    int w, h;
    ValueMap(double *values=0, int w=0, int h=0):values(values), w(w), h(h){}
    double value(double x,double y){return values && w ? values[int(y)*w + int(x)] : 0;}
    SPoint upper_bound(){return SPoint(w-1,h-1);}
    SPoint lower_bound(){return SPoint(0,0);}
};

class SurfaceRaster : public CRaster
{
public:
    SurfaceRaster( const ci::Surface32f& image ) : mImage(image) {}
    
    double value(double x,double y) {
        return mImage.getPixel( ci::Vec2i(x,y) ).r;
    }
    SPoint upper_bound() { return SPoint(mImage.getWidth()-1, mImage.getHeight()-1); }
    SPoint lower_bound() { return SPoint(0,0); }
    
    ci::Surface32f mImage;
};

//
// Contour
//
class Contour : public Scene
{
public:
    Contour();
    virtual ~Contour();
    
    // inherited from Scene
    void setup();
    //void reset();
    //void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
    //HACKS!
    ci::gl::Fbo& getVtfFbo() { return mVtfFbo; }
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void openFile();
    void drawContourLines();
    void updateAudioResponse();
    
    void setupDynamicTexture();
    void drawDynamicTexture();
    void drawFromDynamicTexture();
    
private:
    ci::gl::Texture			mTexture;
    
    // BOURKE
    CContourMap mContourMap;
    float mMinValue;
    float mMaxValue;
    int mLevels;
    
    ci::Vec2f mZoom;
    
    // PERLIN
    ci::gl::Fbo					mVtfFbo;
	ci::gl::GlslProg			mShaderTex;
	float						mDisplacementSpeed;
    float                       mDisplacementHeight;
	float						mTheta;
    ci::gl::GlslProg			mShaderVtf;
    ci::Vec3f                       mNoiseScale;
    
    // 1D texture map
    ci::gl::Fbo					mStripFbo;
    
    // params

};

