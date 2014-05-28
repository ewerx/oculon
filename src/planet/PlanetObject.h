//
//  PlanetObject.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-26.
//
//

#pragma once

#include "CameraController.h"
#include "TimeController.h"
#include "AudioInputHandler.h"

#include "cinder/Cinder.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

class PlanetObject
{
public:
    PlanetObject();
    ~PlanetObject();
    
    void draw();
    
private:
    ci::Vec3f           mPosition;
	float               mRadius;
    
    // mesh
    ci::gl::VboMesh     mMesh;
    
    // textures
	ci::gl::Texture     mTexDiffuse;
	ci::gl::Texture     mTexNormal;
	ci::gl::Texture     mTexMask;
    
    ci::gl::GlslProg    mShader;
    
    ci::Vec3f			mLightDir;
    
};