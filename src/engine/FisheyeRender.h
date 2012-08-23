//
//  FisheyeRender.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-03-07.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#ifndef FISHEYERENDER_H
#define FISHEYERENDER_H

#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Exception.h"
#include "cinder/Surface.h"
#include "cinder/Camera.h"

class FisheyeRender
{
public:
    FisheyeRender( int32_t imageWidth, int32_t imageHeight );
	
	bool		nextTile();
	
	int32_t		getImageWidth() const { return mImageWidth; }
	int32_t		getImageHeight() const { return mImageHeight; }
	float		getImageAspectRatio() const { return mImageWidth / (float)mImageHeight; }
	Area		getCurrentTileArea() const { return mCurrentArea; }
	Surface		getSurface() const { return mSurface; }
	
	void		setMatricesWindow( int32_t windowWidth, int32_t windowHeight );
	void		setMatricesWindow( const Vec2i &windowSize ) { setMatricesWindow( windowSize.x, windowSize.y ); }
	void		setMatricesWindowPersp( int screenWidth, int screenHeight, float fovDegrees = 60.0f, float nearPlane = 1.0f, float farPlane = 1000.0f );
	void		setMatricesWindowPersp( const Vec2i &windowSize, float fovDegrees = 60.0f, float nearPlane = 1.0f, float farPlane = 1000.0f )
    { setMatricesWindowPersp( windowSize.x, windowSize.y, fovDegrees, nearPlane, farPlane ); }
	void		setMatrices( const Camera &camera );
    
	void		frustum( float left, float right, float bottom, float top, float nearPlane, float farPlane );
	void		ortho( float left, float right, float bottom, float top, float nearPlane, float farPlane );
	
protected:
    
    enum eCubeFrustums
    {
        FRUSTUM_LEFT,
        FRUSTUM_RIGHT,
        FRUSTUM_TOP,
        FRUSTUM_BOTTOM,
        FRUSTUM_FRONT,
        
        FRUSTUM_COUNT
    };
    ci::Camera* mCamera[FRUSTUM_COUNT];
    ci::Texture mTexture[FRUSTUM_COUNT];
    
    
	void		updateFrustum();
    
	int32_t		mImageWidth, mImageHeight;
	int32_t		mTileWidth, mTileHeight;
	int32_t		mNumTilesX, mNumTilesY;
    
	int32_t		mCurrentTile;
	Area		mCurrentArea;
	Rectf		mCurrentFrustumCoords;
	float		mCurrentFrustumNear, mCurrentFrustumFar;
	bool		mCurrentFrustumPersp;
	
	Area		mSavedViewport;
	Surface		mSurface;
};
}

#endif
