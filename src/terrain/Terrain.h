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
    const Camera& getCamera();
    
    // new
    
protected:
    // inherited from Scene
    void setupInterface();
    void setupDebugInterface();
    
    // new
    void loadMesh();
    void createMeshes();
    void generate();
    
    const ci::gl::VboMesh& getMesh();
    
private:
    
    ci::gl::Fbo                 mFbo;
    
    ci::gl::VboMesh             mCylinder;
    ci::TriMesh                 mTriMesh;
    
    std::vector<uint32_t> indices;
	std::vector<Vec3f> normals;
	std::vector<Vec3f> positions;
	std::vector<Vec3f> srcNormals;
	std::vector<Vec3f> srcPositions;
	std::vector<Vec2f> srcTexCoords;
	std::vector<Vec2f> texCoords;
    
    int slice;
    
    // Lighting
	ci::gl::Light				*mLight;
	bool						mLightEnabled;
    
    // Texture map
	ci::gl::Texture				mTexture;
	bool						mTextureEnabled;
    
    // Instancing shader
	ci::gl::GlslProg			mShader;
    
    ci::ColorAf                 mColor;
    
	bool						mWireframe;
    bool                        mDrawInstances;
    bool                        mAdditiveBlending;
    bool                        mDynamicLight;
    
    float                       mObjectScale;
    int32_t						mDivision;
    int32_t                     mResolution;
    int32_t                     mSlices;
    int                         mLineWidth;
    
    
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
};

#endif // __TERRAIN_H__