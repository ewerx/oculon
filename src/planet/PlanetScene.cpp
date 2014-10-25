//
//  PlanetScene.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-26.
//
//

#include "PlanetScene.h"
#include "OculonApp.h"
#include "SpinCam.h"
#include "SplineCam.h"
#include "OtherSceneCam.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// ----------------------------------------------------------------
//
PlanetScene::PlanetScene()
: Scene("moon")
{
}

// ----------------------------------------------------------------
//
PlanetScene::~PlanetScene()
{
}

#pragma mark - Setup

// ----------------------------------------------------------------
//
void PlanetScene::setup()
{
    Scene::setup();
    
//    mDynamicTexture.setup(mMeshResolution.x, mMeshResolution.y);
    
    mCameraController.setup(mApp);
    mCameraController.addCamera( new SpinCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new SplineCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new OtherSceneCam(mApp, "graviton") );
    mCameraController.addCamera( new OtherSceneCam(mApp, "parsec") );
    mCameraController.setCamIndex(3);
    
    mBackgroundAlpha = 0.0f;
}

// ----------------------------------------------------------------
//
void PlanetScene::setupInterface()
{
//    mDynamicTexture.setupInterface(mInterface, mName);
    
    mCameraController.setupInterface(mInterface, mName);
}

// ----------------------------------------------------------------
//
void PlanetScene::reset()
{
}

#pragma mark - Update

// ----------------------------------------------------------------
//
void PlanetScene::update(double dt)
{
    Scene::update(dt);
    
    mCameraController.update(dt);
    
//    mDynamicTexture.update(dt);
}

#pragma mark - Draw

const Camera& PlanetScene::getCamera()
{
    return mCameraController.getCamera();
}

void PlanetScene::draw()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    
    glShadeModel( GL_SMOOTH );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	gl::enable( GL_NORMALIZE );
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
    
    glDepthFunc( GL_LESS );
	glEnable( GL_DEPTH_TEST );
    
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
    
    gl::pushMatrices();
    // Set up window
	gl::setViewport( mApp->getViewportBounds() );
	gl::setMatrices( getCamera() );
	
    mPlanet.draw();
    
    gl::popMatrices();
    glPopAttrib();
}