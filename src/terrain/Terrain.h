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
#include "TunnelCam.h"
#include "RDiffusion.h"
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
    void updateAudioResponse();
    
    // HOUX
    void enableLights();
	void disableLights();
    
	void setupShadowMap();
	void renderShadowMap();
    
	bool setupMesh();
    void setupDynamicTexture();
    
    void drawDynamicTexture();
    void drawMesh();
    
private:
    // camera
#define TERRAIN_CAMTYPE_TUPLE \
TERRAIN_CAMTYPE_ENTRY( "Manual", CAM_MANUAL ) \
TERRAIN_CAMTYPE_ENTRY( "Static", CAM_STATIC ) \
TERRAIN_CAMTYPE_ENTRY( "Tunnel", CAM_TUNNEL ) \
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
    TunnelCam                   mTunnelCam;
    float                       mTunnelDistance;
    
    // HOUX
    bool			mDrawWireframe;
	bool			mDrawFlatShaded;
	bool			mDrawShadowMap;
    bool            mEnableLight;
    bool            mEnableTexture;
    bool            mEnableShadow;
	Vec3f			mLightPosition;
	TriMesh			mTriMesh;
	Matrix44f		mShadowMatrix;
	CameraPersp		mShadowCamera;
    
    // BTR
    ci::gl::Light   *mLight;
    
    gl::VboMesh     mVboMesh[2];
    int             mCurMesh;
    int             mNextMesh;
    
	gl::Fbo			mDepthFbo;
	gl::GlslProg	mShader;
	gl::Texture		mTexture;
    
    // mesh type
#define TERRAIN_MESHTYPE_TUPLE \
TERRAIN_MESHTYPE_ENTRY( "Surface", MESHTYPE_FLAT ) \
TERRAIN_MESHTYPE_ENTRY( "Tunnel", MESHTYPE_CYLINDER ) \
//end tuple
    
    enum eMeshType
    {
#define TERRAIN_MESHTYPE_ENTRY( nam, enm ) \
        enm,
        TERRAIN_MESHTYPE_TUPLE
#undef  TERRAIN_MESHTYPE_ENTRY
        
        MESHTYPE_COUNT
    };
    eMeshType   mMeshType;
    
    // Dynamic texture
	ci::gl::Fbo					mVtfFbo;
	ci::gl::GlslProg			mShaderTex;
	float						mDisplacementSpeed;
	float						mTheta;
    //Perlin                      mPerlin;
    
    // Displacement
    ci::Anim<float>             mDisplacementHeight;
	ci::gl::GlslProg			mShaderVtf;
    Vec3f                       mNoiseScale;
    
    // displacement
#define TERRAIN_DISPLACEMODE_TUPLE \
TERRAIN_DISPLACEMODE_ENTRY( "None", DISPLACE_NONE ) \
TERRAIN_DISPLACEMODE_ENTRY( "Noise", DISPLACE_NOISE ) \
TERRAIN_DISPLACEMODE_ENTRY( "Audio", DISPLACE_AUDIO ) \
TERRAIN_DISPLACEMODE_ENTRY( "Diffusion", DISPLACE_RDIFF ) \
//end tuple
    
    enum eDisplacementMode
    {
#define TERRAIN_DISPLACEMODE_ENTRY( nam, enm ) \ 
enm,
        TERRAIN_DISPLACEMODE_TUPLE
#undef  TERRAIN_DISPLACEMODE_ENTRY
        
        DISPLACE_COUNT
    };
    eDisplacementMode   mDisplacementMode;

    RDiffusion          mRDiffusion;
    
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
    
    bool                mAudioEffectNoise;
    
    bool                mInfiniteTunnel;
    
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

#endif // __TERRAIN_H__