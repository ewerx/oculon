//
// Polyhedron.h
// Oculon
//
// Created by Ehsan on 12-01-05.
// Copyright 2012 ewerx. All rights reserved.
//

#ifndef __Polyhedron_H__
#define __Polyhedron_H__

#include "Scene.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/GlslProg.h"

class Polyhedron : public Scene
{
public:
    Polyhedron();
    virtual ~Polyhedron();

    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    
    // new
    
protected:
    // inherited from Scene
    void setupInterface();
    void setupDebugInterface();
    
    // new
    void loadMesh();
    void createMeshes();
    
    void drawInstances();
    void drawInstanced( const ci::gl::VboMesh &vbo, size_t count = 1 );
    
private:
    ci::TriMesh			mMesh;
    ci::gl::VboMesh		mVboMesh;
    ci::gl::Fbo         mFbo;
    
    // Lighting
	ci::gl::Light				*mLight;
	bool						mLightEnabled;
    
    // Texture map
	ci::gl::Texture				mTexture;
	bool						mTextureEnabled;
    
    // Instancing shader
	ci::gl::GlslProg			mShader;
    
    // Instance grid
	ci::Vec3i					mGridSize;
	ci::Vec3f					mGridSpacing;
    
	bool						mWireframe;
    bool                        mDrawInstances;
    
    float                       mObjectScale;
    int32_t						mDivision;
    int32_t                     mResolution;
};

#endif // __Polyhedron_H__