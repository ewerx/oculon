//
//  Deformer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/10/2013.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include "Scene.h"
#include "CameraController.h"
#include "SimplexNoiseTexture.h"

#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

#include <vector>

class Deformer : public Scene
{
public:
    Deformer();
    virtual ~Deformer();
    
    // inherited from Scene
    void setup();
    void reset();
//    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    const ci::Camera& getCamera();
    
protected:
    // inherited from Scene
    void setupInterface();
//    //void setupDebugInterface();
    
    void createMesh();
    
private:
    // mesh
    ci::gl::VboMesh     mMesh;
    ci::Vec2i           mMeshResolution;
    ci::Vec3f           mMeshScale;
    
    // mesh texture
	ci::gl::Texture		mMeshTexture;
	bool				mMeshTextureEnabled;
    bool                mWireframe;
    bool                mLightEnabled;
    ci::gl::GlslProg    mShaderVtf;
    
    // dynamic texture
    SimplexNoiseTexture mDynamicTexture;
    float mDisplacementHeight;

    // camera
    CameraController mCameraController;
};
