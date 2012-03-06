/*
 *  ShaderTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __SHADEREST_H__
#define __SHADEREST_H__

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"

//
// Audio input tests
//
class ShaderTest : public Scene
{
public:
    ShaderTest();
    virtual ~ShaderTest();
    
    // inherited from Scene
    void setup();
    //void setupParams(params::InterfaceGl& params);
    //void reset();
    void update(double dt);
    void draw();
    bool handleKeyDown(const KeyEvent& keyEvent);
    
private:
    enum 
    { 
        FBO_COUNT = 2,
        FBO_ITERATIONS = 4
    };
    
    gl::Fbo             mFbo[FBO_COUNT];
    int                 mFboPing;
    int                 mFboPong;
    
    gl::GlslProg        mShader;
    
    Vec2f               mPos;
    Vec2f               mVel;
    
    bool                mEnableShader;
    float               mBlurAmount;
    
};

#endif // __MOVIETEST_H__
