//
//  Fluid.h
//  Oculon
//
//  Created by Ehsan Rezaie on 1/12/2014.
//
//

#pragma once

#include <vector>
#include "cinder/Cinder.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "Scene.h"
#include "AudioInputHandler.h"
#include "gradient.h"
#include "fluidmodel.h"

//
// Fluid
//
class Fluid : public Scene
{
public:
    Fluid();
    virtual ~Fluid();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    void shutdown();
    
    void handleMouseDown( const ci::app::MouseEvent& mouseEvent );
	void handleMouseDrag( const ci::app::MouseEvent& event );
    
protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();
    
private:
    void resetFluidModel();
    void resetQuadraticSize();
    void updateGradient(int colorScheme);
    void drawFluidModel();
    
private:
    FluidModel*             mFluidModel;
    //color gradient used to visualize the [0..1] density value of the fluid
    Gradient                mGradientMaker;
    Gradient::ColorVector   mGradientColors;
    
    
    // rendering
    ci::gl::GlslProg        mDrawShader;
    ci::gl::Texture         mFluidTexture;
    ci::gl::Texture         mGradientTexture;
    
    bool                    mEnableLighting;
    float                   mViscosity;
    float                   mDiffusion;
    int                     mQuadraticSize;
    float                   mQuadraticPixelSize;
    float                   mSimulationRate;
    
    // mouse
    ci::Vec2i               mPrevMousePos;
    
    // audio
    AudioInputHandler       mAudioInputHandler;
};


