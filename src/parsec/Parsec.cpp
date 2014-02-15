//
//  Parsec.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12/7/2013.
//
//

#include "OculonApp.h"
#include "Parsec.h"
#include "Orbiter.h"

using namespace ci;
using namespace ci::app;
using namespace std;

Parsec::Parsec()
: Scene("parsec")
{
}

Parsec::~Parsec()
{
}

#pragma mark - Setup

void Parsec::setup()
{
    Scene::setup();
    
    // params
    mShowBackground = false;
    mShowGrid = false;
    mLabelsAlpha = 1.0f;
    mShowConstellations = false;
    mShowConstellationArt = false;
    mConstellationAudio = false;
    mConstellationAlpha = 1.0f;
    
    loadData();
    
	mGrid.setup();
    mBackground.setup();
    
    //TODO: refactor
	mStars.setup();
	mStars.setAspectRatio(1.0f);// ( mIsStereoscopic ? 0.5f : 1.0f ); // TODO
    
    if( fs::exists( getAssetPath("") / "parsec-constellations.cdb" ) )
    {
		mConstellations.read( loadFile( getAssetPath("") / "parsec-constellations.cdb" ) );
    }
    mConstellations.setup();
    mConstellations.setLineWidth(3.0f);
    mConstellationArt.setup();
    
    // cylindrical projection settings
	mSectionCount = 3;
	mSectionFovDegrees = 72.0f;
	// for values smaller than 1.0, this will cause each view to overlap the other ones
	//  (angle of overlap: (1 - mSectionOverlap) * mSectionFovDegrees)
	mSectionOverlap = 1.0f;
    
    // cameras
    mCameraController.setup(mApp, CameraController::CAM_MANUAL|CameraController::CAM_SPLINE|CameraController::CAM_GRAVITON, CameraController::CAM_STAR);
    
    mAudioInputHandler.setup(false);
    mAudioInputHandler.mRandomEveryFrame = true;
    mAudioInputHandler.mRandomSignal = true;
}

void Parsec::reset()
{
}

void Parsec::setupInterface()
{
    mInterface->addParam(CreateBoolParam("grid", &mShowGrid));
    mInterface->addParam(CreateBoolParam("background", &mShowBackground));
    mInterface->addParam(CreateFloatParam("labels-alpha", &mLabelsAlpha));
    mInterface->addParam(CreateBoolParam("labels-fadebydist", &mLabels.mFadeByDistance));
    mInterface->addParam(CreateBoolParam("labels-audioresponsive", &mLabels.mAudioResponsive));
    mInterface->addParam(CreateBoolParam("constellations", &mShowConstellations));
//    mInterface->addParam(CreateBoolParam("const art", &mShowConstellationArt));
    
    mInterface->addParam(CreateBoolParam("const audio", &mConstellationAudio));
    mInterface->addParam(CreateFloatParam("const alpha", &mConstellationAlpha));
    
    
    mCameraController.setupInterface(mInterface, getName());
    
    mAudioInputHandler.setupInterface(mInterface, getName());
}

void Parsec::loadData()
{
    // load the star database and create the VBO mesh
//	if( fs::exists( getAssetPath("") / "parsec-stars.cdb" ) )
//    {
//		mStars.read( loadFile( getAssetPath("") / "parsec-stars.cdb" ) );
//    }
//    else
    {
        mStars.load( loadResource( "hygxyz.csv" ), mLabels );
    }
    
//	if( fs::exists( getAssetPath("") / "labels.cdb" ) )
//    {
//		mLabels.read( loadFile( getAssetPath("") / "labels.cdb" ) );
//    }
//    else
//    {
//        mLabels.load( loadFile( getAssetPath("") / "hygxyz.csv" ) );
//    }
//    
//	if( fs::exists( getAssetPath("") / "constellations.cdb" ) )
//    {
//		mConstellations.read( loadFile( getAssetPath("") / "constellations.cdb" ) );
//    }
//    
//	if( fs::exists( getAssetPath("") / "constellationlabels.cdb" ) )
//    {
//		mConstellationLabels.read( loadFile( getAssetPath("") / "constellationlabels.cdb" ) );
//    }
}

#pragma mark - Camera

const ci::Camera& Parsec::getCamera()
{
    return mCameraController.getCamera();
}

#pragma mark - Update

void Parsec::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput(), mGain);
    
    if (mConstellationAudio)
    {
        mConstellationAlpha = 3.0f * mAudioInputHandler.getAverageVolumeHighFreq();
    }
    
    // update camera
    mCameraController.update(dt);
    
    // adjust content based on camera distance
    float distance = getCamera().getEyePoint().length();
	mBackground.setCameraDistance( distance );
    mConstellations.setCameraDistance( distance );
	mConstellationArt.setCameraDistance( distance );
    
    if (mLabelsAlpha > 0.0f)
    {
        mLabels.updateAudio(mAudioInputHandler, mGain);
        mLabels.update(getCamera(), distance, mApp->getViewportWidth(), mApp->getViewportHeight());
    }
    
    mStars.update();
}

#pragma mark - Draw

void Parsec::draw()
{
    //TODO: cylindrical projection
    //TODO: stereoscopic rendering
    
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    render();
    gl::popMatrices();
    
    if (mLabelsAlpha > 0.0f)
    {
        gl::pushMatrices();
        mLabels.draw(mApp->getViewportWidth(), mApp->getViewportHeight(), mLabelsAlpha);
        gl::popMatrices();
    }
}

void Parsec::render()
{
    // draw background
	if(mShowBackground)
    {
        mBackground.draw();
    }
    
	// draw grid
	if(mShowGrid)
    {
		mGrid.draw();
    }
    
	// draw stars
	mStars.draw();
    
	// draw constellations
	if(mShowConstellations)
    {
		mConstellations.draw(mConstellationAlpha);
    }
    
	if(mShowConstellationArt)
    {
		mConstellationArt.draw();
    }
//
//	// draw labels
//	if(mIsLabelsVisible) {
//		mLabels.draw();
//        
//		if(mIsConstellationsVisible || mIsConstellationArtVisible)
//			mConstellationLabels.draw();
//	}
}

void Parsec::drawDebug()
{
    Scene::drawDebug();
}
