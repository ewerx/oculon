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
    mShowLabels = false;
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
    mStarCam.setup(mApp);
    mSplineCam.setup();
    
    //HACK!
    mStarCamTimeScale = 0.005f;
    mStarCam.mTimeScale = &mStarCamTimeScale;
    
    mAudioInputHandler.setup(false);
}

void Parsec::reset()
{
}

void Parsec::setupInterface()
{
    mInterface->addParam(CreateBoolParam("grid", &mShowGrid));
    mInterface->addParam(CreateBoolParam("background", &mShowBackground));
    mInterface->addParam(CreateBoolParam("labels", &mShowLabels));
    mInterface->addParam(CreateBoolParam("constellations", &mShowConstellations));
    mInterface->addParam(CreateBoolParam("const art", &mShowConstellationArt));
    
    
    mInterface->addParam(CreateBoolParam("const audio", &mConstellationAudio));
    mInterface->addParam(CreateFloatParam("const alpha", &mConstellationAlpha));
    
    
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define PARSEC_CAMTYPE_ENTRY( nam, enm ) \
camTypeNames.push_back(nam);
    PARSEC_CAMTYPE_TUPLE
#undef  PARSEC_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "camera", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName())
                        .isVertical(), camTypeNames);

    mInterface->addParam(CreateFloatParam("star cam speed", &mStarCamTimeScale));
    mSplineCam.setupInterface(mInterface, getName());
    
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
    switch( mCamType )
    {
        case CAM_MANUAL:
            return Scene::getCamera();
            
        case CAM_SPLINE:
            return mSplineCam.getCamera();
            
        case CAM_STAR:
            return mStarCam.getCamera();
            
        case CAM_ORBITER:
        {
            Orbiter* orbiterScene = static_cast<Orbiter*>(mApp->getScene("orbiter"));
            
            if( orbiterScene && orbiterScene->isRunning() && orbiterScene->getCamType() != Orbiter::CAM_CATALOG )
            {
                return orbiterScene->getCamera();
            }
            else
            {
                return mStarCam.getCamera();
            }
        }
            
        case CAM_CATALOG:
        {
            Scene* scene = mApp->getScene("catalog");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mStarCam.getCamera();
            }
        }
            
        default:
            return mStarCam.getCamera();
    }
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
    //TODO: method
    switch (mCamType) {
        case CAM_SPLINE:
            mSplineCam.update(dt);
            break;
            
        case CAM_STAR:
            mStarCam.update(dt);
            break;
            
        default:
            break;
    }
    
    // adjust content based on camera distance
    float distance = getCamera().getEyePoint().length();
	mBackground.setCameraDistance( distance );
    mConstellations.setCameraDistance( distance );
	mConstellationArt.setCameraDistance( distance );
    
    if (mShowLabels)
    {
        mLabels.update(getCamera(), mApp->getViewportWidth(), mApp->getViewportHeight());
    }
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
    
    if (mShowLabels)
    {
        gl::pushMatrices();
        mLabels.draw(mApp->getViewportWidth(), mApp->getViewportHeight(), 1.0f);
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
    
}