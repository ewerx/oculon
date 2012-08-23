/*
 *  FisheyeTest.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __FISHEYETEST_H__
#define __FISHEYETEST_H__

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"

//
// Audio input tests
//
class FisheyeTest : public Scene
{
public:
    FisheyeTest();
    virtual ~FisheyeTest();
    
    // inherited from Scene
    void setup();
    //void setupParams(params::InterfaceGl& params);
    //void reset();
    void update(double dt);
    void draw();
    bool handleKeyDown(const KeyEvent& keyEvent);
    
private:
    bool                mUseShader;
    gl::GlslProg        mShader;
    gl::Texture         mTexture;
};

#endif // __FisheyeTest_H__
