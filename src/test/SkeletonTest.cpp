/*
 *  SkeletonTest.cpp
 *  Oculon
 *
 *  Created by Ehsan on 12-03-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "OculonApp.h"
#include "SkeletonTest.h"
#include "Resources.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/CinderMath.h"
#include <boost/format.hpp>


using namespace ci;

SkeletonTest::SkeletonTest()
{
}

SkeletonTest::~SkeletonTest()
{
    m_2RealKinect->shutdown();
}

void SkeletonTest::setup()
{
    m_iKinectWidth = 640;
	m_iKinectHeight = 480;
	m_iScreenWidth = mApp->getWindowWidth();
	m_iScreenHeight = mApp->getWindowHeight();
	m_bIsMirroring = true;	
	m_iMotorValue = 0;
    
	try
	{
		m_2RealKinect = _2RealKinect::getInstance();
		std::cout << "_2RealKinectWrapper Version: " << m_2RealKinect->getVersion() << std::endl;
		bool bResult = m_2RealKinect->start( COLORIMAGE | DEPTHIMAGE | USERIMAGE );
		if( bResult )
			std::cout << "\n\n_2RealKinectWrapper started successfully!...";
		
		m_iNumberOfDevices = m_2RealKinect->getNumberOfDevices();
		m_iMotorValue = m_2RealKinect->getMotorAngle(0);	// just make motor device 0 controllable
		resizeImages();
	}
	catch ( std::exception& e )
	{
		std::cout << "\n\n_2Real: Exception occurred when trying to start:\n" << e.what() << std::endl;
		m_2RealKinect->shutdown();
		int pause = 0;
		std::cin >> pause;
	}
}

void SkeletonTest::update(double dt)
{
}

void SkeletonTest::draw()
{
    gl::pushMatrices();
    gl::setMatricesWindowPersp(getWindowSize());

    try
	{
		gl::clear();
		gl::color( Color(1,1,1) );
		drawKinectImages();				//oututs all connected kinect devices generators (depth, rgb, user imgage, skeletons)			
	}
	catch( std::exception& e )
	{
		std::cout << "\nException in draw: " << e.what() << std::endl;
	}
    
    gl::popMatrices();
}

void SkeletonTest::drawDebug()
{
    char buf[256];
    const Vec3f& pos = Vec3f::zero();
    snprintf(buf, 256, "tracking: %.1f,%.1f,%.1f", pos.x, pos.y, pos.z );
    mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.7f, 0.8f));
}

bool SkeletonTest::handleKeyDown(const KeyEvent& keyEvent)
{    
    bool handled = true;
    
    if( keyEvent.getChar() == 'm' )	// mirror generators
		mirrorImages();
	if( keyEvent.getChar() == 'r' )	// restart kinects
	{
		bool bResult = m_2RealKinect->restart();
		if( bResult )
			std::cout << "\n\n_2Real started successfully!...";
		m_iNumberOfDevices = m_2RealKinect->getNumberOfDevices();
	}
	if( keyEvent.getChar() == 'u' )	// reset all calibrated users (OpenNI only)
	{
		m_2RealKinect->resetAllSkeletons();
	}
	if( keyEvent.getCode() == ci::app::KeyEvent::KEY_UP )	
	{
		m_iMotorValue+=3;
		if(m_2RealKinect->setMotorAngle(0, m_iMotorValue))
			m_iMotorValue = m_2RealKinect->getMotorAngle(0);
		else
			m_iMotorValue-=3;
	}
	if( keyEvent.getCode() == ci::app::KeyEvent::KEY_DOWN )	
	{
		m_iMotorValue-=3;
		if(m_2RealKinect->setMotorAngle(0, m_iMotorValue))
			m_iMotorValue = m_2RealKinect->getMotorAngle(0);
		else
			m_iMotorValue+=3;
	}
    
    switch (keyEvent.getCode()) 
    {
        default:
            handled = false;
            break;
    }
    
    return handled;    
}

void SkeletonTest::drawKinectImages()
{			
	unsigned char* imgRef;
	int numberChannels = 0;
    
	cout << m_iMotorValue << std::endl;
	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		ci::Rectf destinationRectangle( m_ImageSize.x * i, 0, m_ImageSize.x * (i+1), m_ImageSize.y);
		
		//rgb image
		imgRef = getImageData( i, COLORIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
		Surface8u color( imgRef, m_iKinectWidth, m_iKinectHeight, m_iKinectWidth*numberChannels, SurfaceChannelOrder::RGB );
		gl::draw( gl::Texture( color ), destinationRectangle);
		
		//depth image		
		imgRef = getImageData( i, DEPTHIMAGE, m_iKinectWidth, m_iKinectHeight, numberChannels);
		Channel depth( m_iKinectWidth, m_iKinectHeight, m_iKinectWidth, numberChannels, imgRef );
		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
		gl::draw( gl::Texture( depth ),  destinationRectangle );		
		
		//user image
#ifdef TARGET_MSKINECTSDK
		if( i == 0 )						
#endif
		{
			imgRef = getImageData( i, USERIMAGE_COLORED, m_iKinectWidth, m_iKinectHeight, numberChannels);
			Surface8u userColored( imgRef, m_iKinectWidth, m_iKinectHeight, m_iKinectWidth*3, SurfaceChannelOrder::RGB );
			destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );			
			gl::draw( gl::Texture( userColored ), destinationRectangle );
		}
		
		//skeleton		
		m_iKinectWidth = m_2RealKinect->getImageWidth( i, COLORIMAGE );		
		m_iKinectHeight = m_2RealKinect->getImageHeight( i, COLORIMAGE );
		destinationRectangle.offset( ci::Vec2f( 0, m_ImageSize.y) );
		drawSkeletons(i, destinationRectangle );
		
		//drawing debug strings for devices
		gl::disableDepthRead();
		gl::color(Color( 1.0, 1.0, 1.0 ));	
		gl::drawString( "Device "+ toString(i), Vec2f( m_ImageSize.x * i + 20 , 0 ), Color( 1.0f, 0.0f, 0.0f ), m_Font );		
		gl::enableDepthRead();
	}
}

unsigned char* SkeletonTest::getImageData( int deviceID, _2RealGenerator imageType, int& imageWidth, int& imageHeight, int& bytePerPixel )
{
	bytePerPixel = m_2RealKinect->getBytesPerPixel( imageType );
	imageWidth = m_2RealKinect->getImageWidth( deviceID, imageType );		
	imageHeight = m_2RealKinect->getImageHeight( deviceID, imageType );
    
	return m_2RealKinect->getImageData( deviceID, imageType);
}

void SkeletonTest::drawSkeletons(int deviceID, ci::Rectf rect)
{
	float fRadius = 10.0;
    
	gl::pushMatrices();
	try
	{
		glLineWidth(2.0);
		
        
		glTranslatef( rect.getX1(), rect.getY1(), 0 );
		glScalef( rect.getWidth()/(float)m_iKinectWidth, rect.getHeight()/(float)m_iKinectHeight, 1);
        
		_2RealPositionsVector2f::iterator iter;
        
        
		for( unsigned int i = 0; i < m_2RealKinect->getNumberOfUsers( deviceID ); ++i)
		{		
			glColor3f( 0, 1.0, 0.0 );				
			_2RealPositionsVector2f skeletonPositions = m_2RealKinect->getSkeletonScreenPositions( deviceID, i );
            
			_2RealOrientationsMatrix3x3 skeletonOrientations;
			if(m_2RealKinect->hasFeatureJointOrientation())
				skeletonOrientations = m_2RealKinect->getSkeletonWorldOrientations( deviceID, i );
            
			int size = skeletonPositions.size();		
			for(int j = 0; j < size; ++j)
			{	
				_2RealConfidence jointConfidence = m_2RealKinect->getSkeletonJointConfidence(deviceID, i, _2RealJointType(j));
				gl::pushModelView();
				if( m_2RealKinect->isJointAvailable( (_2RealJointType)j ) && jointConfidence.positionConfidence > 0.0)
				{
					glTranslatef(Vec3f( skeletonPositions[j].x, skeletonPositions[j].y, 0 ));
                    
					if(m_2RealKinect->hasFeatureJointOrientation() && jointConfidence.orientationConfidence > 0.0)
					{
						Matrix44<float> rotMat  = gl::getModelView();
						rotMat.m00 = skeletonOrientations[j].elements[0];
						rotMat.m01 = skeletonOrientations[j].elements[1];
						rotMat.m02 = skeletonOrientations[j].elements[2];
						rotMat.m10 = skeletonOrientations[j].elements[3];
						rotMat.m11 = skeletonOrientations[j].elements[4];
						rotMat.m12 = skeletonOrientations[j].elements[5];
						rotMat.m20 = skeletonOrientations[j].elements[6];
						rotMat.m21 = skeletonOrientations[j].elements[7];
						rotMat.m22 = skeletonOrientations[j].elements[8];
						glLoadMatrixf(rotMat);		
						gl::drawCoordinateFrame(fRadius);
					}
					else
					{
						gl::drawSolidCircle( Vec2f( 0, 0 ), fRadius);
					}
				}
				gl::popModelView();
			}
		}	
		glPopMatrix();
		glLineWidth(1.0);
	}
	catch(...)
	{
	}
	gl::popMatrices();
}

void SkeletonTest::resize()
{
	m_iScreenWidth = mApp->getWindowWidth();
	m_iScreenHeight = mApp->getWindowHeight();
	resizeImages();
}

void SkeletonTest::resizeImages()
{
	//calculate imagesize
	int iImageHeight = (int)(m_iScreenHeight / 4.0);		// divide window height according to the number of generator outputs (rgb, depth, user, skeleton)
	int iImageWidth = (int)(iImageHeight * 4.0 / 3.0);		// keep images aspect ratio 4:3
	if(iImageWidth * m_iNumberOfDevices > m_iScreenWidth)	// aspect ratio 	
	{
		iImageWidth = m_iScreenWidth / m_iNumberOfDevices;
		iImageHeight = iImageWidth * 3 / 4;
	}
	//size of plane to draw textures on
	m_ImageSize = Vec2f( (float)iImageWidth, (float)iImageHeight );
}

void SkeletonTest::mirrorImages()
{
	m_bIsMirroring = !m_bIsMirroring;	// toggleMirroring
	for( int i = 0; i < m_iNumberOfDevices; ++i)
	{
		m_2RealKinect->setMirrored( i, COLORIMAGE, m_bIsMirroring );
		m_2RealKinect->setMirrored( i, DEPTHIMAGE, m_bIsMirroring );
		m_2RealKinect->setMirrored( i, USERIMAGE, m_bIsMirroring );		// OpenNI has no capability yet to mirror the user image
	}		
}
