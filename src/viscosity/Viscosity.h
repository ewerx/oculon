//
//  Viscosity.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#pragma once

#include "Scene.h"
#include "Fluid2D.h"

#include "cinder/gl/Texture.h"

//
// Viscosity
//
class Viscosity : public Scene
{
public:
    Viscosity();
    virtual ~Viscosity();
    
    // inherited from Scene
    void setup();
    void reset();
    void update(double dt);
    void draw();
    void drawDebug();
    
    void handleMouseDown( const ci::app::MouseEvent& event );
	void handleMouseDrag( const ci::app::MouseEvent& event );
    
    const ci::Camera& getCamera();

protected:// from Scene
    void setupInterface();
    //void setupDebugInterface();

private:
    ci::CameraOrtho         mCamera;
    
    cinderfx::Fluid2D		mFluid2D;
    ci::gl::Texture         mTex;
    
    ci::Vec2f				mPrevPos;
};