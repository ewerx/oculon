//
// Polyhedron.h
// Oculon
//
// Created by Ehsan on 12-01-05.
// Copyright 2012 ewerx. All rights reserved.
//

#pragma once

#include "Scene.h"
#include "SplineCam.h"
#include "AudioInputHandler.h"
#include "SimplexNoiseTexture.h"
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
    void drawDebug();
    const ci::Camera& getCamera();
    
    // new
    
protected:
    // inherited from Scene
    void setupInterface();
    //void setupDebugInterface();
    
    void updateAudioResponse();
    
    // new
    void loadMesh();
    void createMeshes();
    
    void drawInstances( const ci::gl::VboMesh &mesh );
    void drawInstanced( const ci::gl::VboMesh &vbo, size_t count = 1 );
    const ci::gl::VboMesh& getMesh();
    
    // callbacks
    bool onGridSizeChange();
    
private:
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
    int                         mNumObjects;
    
    ci::ColorAf                 mColor;
    
	bool						mWireframe;
    bool                        mDrawInstances;
    bool                        mAdditiveBlending;
    bool                        mDynamicLight;
    
    float                       mObjectScale;
    int32_t						mDivision;
    int32_t                     mResolution;
    int                         mLineWidth;
    
    // meshes
    typedef std::pair<std::string, ci::gl::VboMesh> tNamedMesh;
    std::vector<tNamedMesh>     mMeshes;
    int                         mMeshType;
    
    // noise
    SimplexNoiseTexture         mDynamicTexture;
    
    // camera
    CameraController            mCameraController;
    
    // audio
    AudioInputHandler           mAudioInputHandler;
};
