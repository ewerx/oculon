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
    void drawDebug();
    const ci::Camera& getCamera();
    
    // new
    
protected:
    // inherited from Scene
    void setupInterface();
    void setupDebugInterface();
    
    void updateAudioResponse();
    
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
#define POLYHEDRON_MESHTYPE_TUPLE \
POLYHEDRON_MESHTYPE_ENTRY( "Cube", MESH_TYPE_CUBE ) \
POLYHEDRON_MESHTYPE_ENTRY( "Sphere", MESH_TYPE_SPHERE ) \
POLYHEDRON_MESHTYPE_ENTRY( "Icosahedron", MESH_TYPE_ICOSAHEDRON ) \
//end tuple
    enum eMeshType
    {
#define POLYHEDRON_MESHTYPE_ENTRY( nam, enm ) \
        enm,
        POLYHEDRON_MESHTYPE_TUPLE
#undef  POLYHEDRON_MESHTYPE_ENTRY
        
        MESH_COUNT
    };
    eMeshType                   mMeshType;
    
    // camera
#define POLYHEDRON_CAMTYPE_TUPLE \
POLYHEDRON_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
POLYHEDRON_CAMTYPE_ENTRY( "Orbiter", CAM_ORBITER ) \
POLYHEDRON_CAMTYPE_ENTRY( "Graviton", CAM_GRAVITON ) \
POLYHEDRON_CAMTYPE_ENTRY( "Catalog", CAM_CATALOG ) \
POLYHEDRON_CAMTYPE_ENTRY( "Spline", CAM_SPLINE ) \
//end tuple
    
    enum eCamType
    {
#define POLYHEDRON_CAMTYPE_ENTRY( nam, enm ) \
        enm,
        POLYHEDRON_CAMTYPE_TUPLE
#undef  POLYHEDRON_CAMTYPE_ENTRY
        
        CAM_COUNT
    };
    eCamType                    mCamType;
    
    SplineCam                   mSplineCam;
    
    // AUDIO
    int                 mAudioFboDim;
    ci::Vec2f           mAudioFboSize;
    ci::Area            mAudioFboBounds;
    ci::gl::Fbo         mAudioFbo;
    std::vector< ci::Anim<float> >    mFftFalloff;
    float               mFalloff;
    int                 mAudioRowShift;
    float               mAudioRowShiftTime;
    float               mAudioRowShiftDelay;
    float               mDisplacementScale;
    
    typedef std::function<float (float)> tEaseFn;
    tEaseFn getFalloffFunction();
    tEaseFn getReverseFalloffFunction();
    
    enum eFalloffMode
    {
        FALLOFF_LINEAR,
        FALLOFF_OUTQUAD,
        FALLOFF_OUTEXPO,
        FALLOFF_OUTBACK,
        FALLOFF_OUTBOUNCE,
        FALLOFF_OUTINEXPO,
        FALLOFF_OUTINBACK,
        
        FALLOFF_COUNT
    };
    eFalloffMode mFalloffMode;
};

#endif // __Polyhedron_H__