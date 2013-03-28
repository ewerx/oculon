//
// Terrain.h
// Oculon
//
// Created by Ehsan on 13-Mar-25.
// Copyright 2013 ewerx. All rights reserved.
//


#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "Scene.h"
#include "SplineCam.h"
#include "cinder/Perlin.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/GlslProg.h"

#include <vector>

class Terrain : public Scene
{
public:
    Terrain();
    virtual ~Terrain();

    // inherited from Scene
    void setup();
    void reset();
    void resize();
    void update(double dt);
    void draw();
    void drawDebug();
    const Camera& getCamera();
    
    // new
    
protected:
    // inherited from Scene
    void setupInterface();
    void setupDebugInterface();
    
    // HOUX
    void enableLights();
	void disableLights();
    
	void setupShadowMap();
	void renderShadowMap();
    
	void setupMesh();
    void setupDynamicTexture();
    void updateMesh();
    
    void drawDynamicTexture();
    void drawMesh();
    
    void generateNormals( TriMesh& triMesh );
    
private:
    // camera
#define TERRAIN_CAMTYPE_TUPLE \
TERRAIN_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
TERRAIN_CAMTYPE_ENTRY( "Static", CAM_STATIC ) \
TERRAIN_CAMTYPE_ENTRY( "Orbiter", CAM_ORBITER ) \
TERRAIN_CAMTYPE_ENTRY( "Graviton", CAM_GRAVITON ) \
TERRAIN_CAMTYPE_ENTRY( "Catalog", CAM_CATALOG ) \
TERRAIN_CAMTYPE_ENTRY( "Spline", CAM_SPLINE ) \
//end tuple
    
    enum eCamType
    {
#define TERRAIN_CAMTYPE_ENTRY( nam, enm ) \
        enm,
        TERRAIN_CAMTYPE_TUPLE
#undef  TERRAIN_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    eCamType                    mCamType;
    
    SplineCam                   mSplineCam;
    ci::CameraPersp             mStaticCam;
    
    // HOUX
    bool			mDrawWireframe;
	bool			mDrawFlatShaded;
	bool			mDrawShadowMap;
    bool            mEnableLight;
    bool            mEnableShadow;
    
	Vec3f			mLightPosition;
    
	TriMesh			mTriMesh;
    gl::VboMesh     mVboMesh;
    
	gl::Fbo			mDepthFbo;
	gl::GlslProg	mShader;
	gl::Texture		mTexture;
    
	Matrix44f		mShadowMatrix;
	CameraPersp		mShadowCamera;
    
    enum eMeshType
    {
        MESHTYPE_RANDOM,
        MESHTYPE_SMOOTH,
        MESHTYPE_PERLIN,
        MESHTYPE_FLAT
    };
    int             mMeshType;
    
    // Dynamic texture
	ci::gl::Fbo					mVtfFbo;
	ci::gl::GlslProg			mShaderTex;
	float						mDisplacementSpeed;
	float						mTheta;
    Perlin                      mPerlin;
    
    // Displacement
	float						mDisplacementHeight;
	ci::gl::GlslProg			mShaderVtf;
};

#endif // __TERRAIN_H__