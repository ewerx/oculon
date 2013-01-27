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
#include "SplineCam.h"
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
    const Camera& getCamera();
    
    // new
    
protected:
    // inherited from Scene
    void setupInterface();
    void setupDebugInterface();
    
    // new
    void loadMesh();
    void createMeshes();
    
    void drawInstances( const ci::gl::VboMesh &mesh );
    void drawInstanced( const ci::gl::VboMesh &vbo, size_t count = 1 );
    const ci::gl::VboMesh& getMesh();
    
private:
    //ci::gl::VboMesh             mTriMesh;
    ci::gl::VboMesh				mCircle;
	ci::gl::VboMesh				mCone;
	ci::gl::VboMesh				mCube;
	//ci::gl::VboMesh				mCustom;
	ci::gl::VboMesh				mCylinder;
	ci::gl::VboMesh				mIcosahedron;
	ci::gl::VboMesh				mRing;
	ci::gl::VboMesh				mSphere;
	ci::gl::VboMesh				mSquare;
	ci::gl::VboMesh				mTorus;
    
    ci::gl::Fbo                 mFbo;
    
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
    enum eMeshType
    {
        MESH_TYPE_CUBE,
		MESH_TYPE_SPHERE,
		//MESH_TYPE_CYLINDER,
		//MESH_TYPE_CONE,
        //MESH_TYPE_TORUS,
		MESH_TYPE_ICOSAHEDRON,
		//MESH_TYPE_CIRCLE,
		//MESH_TYPE_SQUARE,
		//MESH_TYPE_RING,
		//MESH_TYPE_CUSTOM,
        
        MESH_COUNT
    };
    eMeshType                   mMeshType;
    
    // camera
    enum eCamType
    {
        CAM_MANUAL,
        CAM_ORBITER,
        CAM_GRAVITON,
        CAM_CATALOG,
        CAM_SPLINE,
        
        CAM_COUNT
    };
    eCamType                    mCamType;
    
    SplineCam                   mSplineCam;
};

#endif // __Polyhedron_H__