//
//  Deformer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 11/10/2013.
//  Copyright 2013 ewerx. All rights reserved.
//

#ifndef __Oculon__Deformer__
#define __Oculon__Deformer__

#include "Scene.h"
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
//    const ci::Camera& getCamera();
    
protected:
    // inherited from Scene
    void setupInterface();
//    //void setupDebugInterface();
    
    void createMesh();
    void generateDeformationTexture();
    
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
    
    // VTF
    ci::gl::GlslProg    mShaderVtf;
    
    // deformation texture
    ci::gl::Fbo         mVtfFbo;
    ci::gl::GlslProg    mShaderTex;
    float               mDisplacementSpeed;
    float               mDisplacementHeight;
    float               mDeformTheta;

};

#endif /* defined(__Oculon__Deformer__) */
