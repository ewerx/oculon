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
    
    
protected:
    void loadMesh();
    void createMeshes();
    
    void setupInterface();
    void setupDebugInterface();
    
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
    
    
	bool						mWireframe;

};

#endif // __Polyhedron_H__