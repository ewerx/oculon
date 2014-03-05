/*
 *  MotionBlurRenderer.h
 *  Oculon
 *
 *  Created by Ehsan on 12-Apr-03.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __MotionBlurRenderer_H__
#define __MotionBlurRenderer_H__

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"


class MotionBlurRenderer
{
public:
    typedef std::function<void(void)> tDrawCallback;
    
public:
    MotionBlurRenderer();
    virtual ~MotionBlurRenderer();
    
    void setup(const ci::Vec2i& windowSize, tDrawCallback drawCallback);
    void resize(const ci::Vec2i& windowSize);
    void update();
    void preDraw();
    void draw();
    
private:

    static const int    FBO_COUNT = 4;
    
    ci::gl::Fbo         mFbo[FBO_COUNT];
    int                 mFboIndex;
    
    ci::gl::GlslProg    mShader;
    
    tDrawCallback       mDrawSceneCallback;
    ci::Vec2i           mWindowSize;
    
};

#endif // __MOVIETEST_H__
