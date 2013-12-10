//
//  Parsec.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12/7/2013.
//
//

#include "OculonApp.h"
#include "Parsec.h"

using namespace ci;
using namespace ci::app;

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
    
    
    loadData();
    
	mGrid.setup();
    mBackground.setup();
	mStars.setup();
	mStars.setAspectRatio(1.0f);// ( mIsStereoscopic ? 0.5f : 1.0f ); // TODO
    
    // cylindrical projection settings
	mSectionCount = 3;
	mSectionFovDegrees = 72.0f;
	// for values smaller than 1.0, this will cause each view to overlap the other ones
	//  (angle of overlap: (1 - mSectionOverlap) * mSectionFovDegrees)
	mSectionOverlap = 1.0f;
}

void Parsec::reset()
{
}

void Parsec::setupInterface()
{
    
}

void Parsec::loadData()
{
    // load the star database and create the VBO mesh
	if( fs::exists( getAssetPath("") / "parsec-stars.cdb" ) )
    {
		mStars.read( loadFile( getAssetPath("") / "parsec-stars.cdb" ) );
    }
    else
    {
        mStars.load( loadResource( "starData.csv" ) );
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
    return mStarCam.getCamera();
//    return Scene::getCamera();
}

#pragma mark - Update

void Parsec::update(double dt)
{
    Scene::update(dt);
    
    // adjust content based on camera distance
    float distance = getCamera().getEyePoint().length();
	mBackground.setCameraDistance( distance );
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
    
//	// draw constellations
//	if(mIsConstellationsVisible)
//		mConstellations.draw();
//    
//	if(mIsConstellationArtVisible)
//		mConstellationArt.draw();
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