/*
 *  BlobTracker.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "BlobTracker.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "KinectController.h"
#include "CinderOpenCv.h"
#include <vector>

using namespace ci;


BlobTracker::BlobTracker()
: mKinectController(NULL)
{
}

BlobTracker::~BlobTracker()
{
}

void BlobTracker::setup(KinectController& kinectController)
{
    mKinectController = &kinectController;
    
    mThreshold = 70.0f;
    mBlobMin = 20.0f;
    mBlobMax = 80.0f;
    
    mTargetPosition = Vec3f::zero();
}


void BlobTracker::update()
{
    if( mKinectController == NULL )
    {
        return;
    }

    // based on http://forum.libcinder.org/topic/simple-hand-tracking-with-kinect-opencv
    Surface depthSurface = mKinectController->getDepthSurface();
    
    if( depthSurface )
    {
        // once the surface is avalable pass it to opencv
        // had trouble here with bit depth. surface comes in full color, needed to crush it down
        cv::Mat input( toOcv( Channel8u( depthSurface )  ) );
        cv::Mat blurred;
        cv::Mat thresholded;
        cv::Mat thresholded2;
        cv::Mat output;
        
        cv::blur(input, blurred, cv::Size(10,10));
        
        // make two thresholded images one to display and one
        // to pass to find contours since its process alters the image
        cv::threshold( blurred, thresholded, mThreshold, 255, CV_THRESH_BINARY);
        cv::threshold( blurred, thresholded2, mThreshold, 255, CV_THRESH_BINARY);
        
        // 2d vector to store the found contours
        vector<vector<cv::Point> > contours;
        // find em
        cv::findContours(thresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        
        // convert theshold image to color for output
        // so we can draw blobs on it
        cv::cvtColor( thresholded2, output, CV_GRAY2RGB );
        
        // loop the stored contours
        for (vector<vector<cv::Point> >::iterator it=contours.begin() ; it < contours.end(); it++ )
        {
            // center abd radius for current blob
            cv::Point2f center;
            float radius;
            // convert the cuntour point to a matrix 
            vector<cv::Point> pts = *it;
            cv::Mat pointsMatrix = cv::Mat(pts);
            // pass to min enclosing circle to make the blob 
            cv::minEnclosingCircle(pointsMatrix, center, radius);
            
            cv::Scalar color( 0, 255, 0 );
            
            if (radius > mBlobMin && radius < mBlobMax) 
            {
                // draw the blob if it's in range
                cv::circle(output, center, radius, color);
                
                //update the target position
                mTargetPosition.x = 640 - center.x;
                mTargetPosition.y = center.y;
                mTargetPosition.z = 0;
            }
        }
        
        mCvTexture = gl::Texture( fromOcv( output ) );
    }
}

