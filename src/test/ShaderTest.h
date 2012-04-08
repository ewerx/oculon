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
#include "MotionBlurRenderer.h"

#include "OscMessage.h"
#include "Interface.h"

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
    //void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseDrag( const ci::app::MouseEvent& event );
    void handleOscMessage( const ci::osc::Message& message );
    
private:
    void updateBlur();
    void drawScene();
    
private:

    enum 
    { 
        FBO_COUNT = 4,
    };
    
    ci::gl::Fbo         mFbo[FBO_COUNT];
    int                 mFboIndex;
    
    ci::gl::GlslProg    mShader;
    
    Vec2f               mPos;
    Vec2f               mVel;

    bool                mUseFbo;
    float               mBlurAmount;
    gl::Texture         mTexture;
    
    MotionBlurRenderer  mMotionBlurRenderer;
    
    //TEST
    Interface*          mInterface;
    float               mRed;
};

#endif // __MOVIETEST_H__
