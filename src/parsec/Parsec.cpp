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
    
    loadData();
    
	mGrid.setup();
    mBackground.setup();
	mStars.setup();
	mStars.setAspectRatio(1.0f);// ( mIsStereoscopic ? 0.5f : 1.0f ); // TODO
    
    mLines.setup(mStars.mBrightStars);
    
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
    mCameraController.setup(mApp, CameraController::CAM_MANUAL|CameraController::CAM_SPLINE, CameraController::CAM_SPLINE);
    
    mAudioInputHandler.setup(false);
    mAudioInputHandler.mRandomEveryFrame = true;
    mAudioInputHandler.mRandomSignal = true;
    
    // params
    mShowGrid = false;
    mShowConstellations = false;
    mShowConstellationArt = false;
    mConstellationAudio = false;
    mConstellationAlpha = 1.0f;
    
    mLabels.mAlpha = 1.0f;
    mLabels.mFadeByDistance = true;
    mLabels.mAudioResponsive = true;
    mBackground.mAlpha = 0.75f;
    
}

void Parsec::reset()
{
}

void Parsec::setupInterface()
{
    mInterface->addParam(CreateBoolParam("grid", &mShowGrid));
    
    mInterface->addParam(CreateFloatParam("star-scale", &mStars.mScale)
                         .minValue(0.5f)
                         .maxValue(3.0f));
    mInterface->addParam(CreateFloatParam("background", &mBackground.mAlpha));
    mInterface->addParam(CreateFloatParam("labels-alpha", &mLabels.mAlpha)
                         .midiInput(0, 2, 22));
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
    if (mApp->outputToOculus())
    {
        return mApp->getOculusCam().getCamera();
    }
    else
    {
        return mCameraController.getCamera();
    }
}

#pragma mark - Update

void Parsec::update(double dt)
{
    Scene::update(dt);
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
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
    
    if (mLabels.mAlpha > 0.0f)
    {
        //TODO: refactor
        mLabels.updateAudio(mAudioInputHandler, mAudioInputHandler.getGain());
        mLabels.update(getCamera(), distance, mApp->getViewportWidth(), mApp->getViewportHeight());
    }
    
    mStars.update();
    mLines.update();
}

#pragma mark - Draw

void Parsec::draw()
{
    //TODO: cylindrical projection
    
    gl::pushMatrices();
    if (mApp->outputToOculus())
    {
        // render left eye
        Area leftViewport = Area( Vec2f( 0.0f, 0.0f ), Vec2f( getFbo().getWidth() / 2.0f, getFbo().getHeight() ) );
        gl::setViewport(leftViewport);
        mApp->getOculusCam().enableStereoLeft();
        gl::setMatrices( mApp->getOculusCam().getCamera() );
        render();
        
        if (mLabels.mAlpha > 0.0f)
        {
            mLabels.draw(leftViewport.getWidth(), leftViewport.getHeight());
        }
        
        // render right eye
        Area rightViewport = Area( Area( Vec2f( getFbo().getWidth() / 2.0f, 0.0f ), Vec2f( getFbo().getWidth(), getFbo().getHeight() ) ) );
        gl::setViewport(rightViewport);
        mApp->getOculusCam().enableStereoRight();
        gl::setMatrices( mApp->getOculusCam().getCamera() );
        render();
        
        if (mLabels.mAlpha > 0.0f)
        {
            mLabels.draw(rightViewport.getWidth(), rightViewport.getHeight());
        }
    }
    else
    {
        gl::setMatrices( getCamera() );
        render();
        
        if (mLabels.mAlpha > 0.0f)
        {
            gl::pushMatrices();
            mLabels.draw(mApp->getViewportWidth(), mApp->getViewportHeight());
            gl::popMatrices();
        }
    }
    gl::popMatrices();
    
}

void Parsec::render()
{
    // draw background
	mBackground.draw();
    
	// draw grid
	if(mShowGrid)
    {
		mGrid.draw();
    }
    
	// draw stars
	mStars.draw();
    
    const float scale = mApp->getViewportWidth() / 128.0f;
    glScalef(scale, scale, scale);
    mLines.draw();
    
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
