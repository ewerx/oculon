//
//  MeshInstanceRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-18.
//
//

#pragma once

#include "ParticleRenderer.h"
#include "AudioInputHandler.h"
#include "cinder/gl/Texture.h"


class MeshInstanceRenderer : public ParticleRenderer
{
public:
    MeshInstanceRenderer();
    virtual ~MeshInstanceRenderer();
    
    // from ParticleRenderer
    void setup(int fboSize);
    void setupInterface( Interface* interface, const std::string& prefix );
    void draw( PingPongFbo& particlesFbo, const ci::Vec2i& screenSize, const ci::Camera& cam, AudioInputHandler& audioInputHandler );
    
protected:
    void preRender();
    void postRender();
    
    void createMeshes();
    const ci::gl::VboMesh& getMesh();
    void drawInstanced( const ci::gl::VboMesh &vbo, size_t count );
    
protected:
    // textures
    typedef std::pair<std::string, ci::gl::Texture> tNamedTexture;
    std::vector<tNamedTexture> mPointTextures;
    int mCurPointTexture;
    
    // mesh
    typedef std::pair<std::string, ci::gl::VboMesh> tNamedMesh;
    std::vector<tNamedMesh>     mMeshes;
    int                         mMeshType;
    
    // params
    bool				mAdditiveBlending;
    ci::ColorAf         mColor;
    bool                mAudioReactive;
    bool                mWireframe;
};