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
    void updateMesh();
    
    void generateNormals( TriMesh& triMesh );
    
private:
    // camera
#define TERRAIN_CAMTYPE_TUPLE \
TERRAIN_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
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
    
    // HOUX
    bool			mDrawWireframe;
	bool			mDrawFlatShaded;
	bool			mDrawShadowMap;
    bool            mEnableLight;
    
	Vec3f			mLightPosition;
    
	TriMesh			mTriMesh;
    
	gl::Fbo			mDepthFbo;
	gl::GlslProg	mShader;
	gl::Texture		mTexture;
    
	Matrix44f		mShadowMatrix;
	CameraPersp		mShadowCamera;
    
    enum eMeshType
    {
        MESHTYPE_RANDOM,
        MESHTYPE_SMOOTH,
        MESHTYPE_PERLIN
    };
    int             mMeshType;
    
};

#endif // __TERRAIN_H__