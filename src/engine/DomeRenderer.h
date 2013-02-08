//
//  DomeRenderer.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-01-27.
//
//

#ifndef __Oculon__DomeRenderer__
#define __Oculon__DomeRenderer__

#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Fbo.h"


class DomeRenderer
{
public:
    DomeRenderer();
    ~DomeRenderer() {};
    
    void setup( int width, int height );
    
    ci::gl::Fbo& getDomeProjectionFbo() { return mDomeFbo; }
    void renderFboToDome( ci::gl::Fbo& fbo, const ci::Camera& camera );
private:
    ci::gl::VboMesh				mSphere;
    ci::gl::Fbo                 mDomeFbo;
    ci::Vec3f                   mScale;
};

#endif /* defined(__Oculon__DomeRenderer__) */
