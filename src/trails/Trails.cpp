//
//  Trails.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-28.
//
//


#include "Trails.h"
#include "Interface.h"
#include "OculonApp.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define TRAIL_LENGTH	16000

Trails::Trails()
: Scene("trails")
{
}

Trails::~Trails()
{
}

void Trails::setup()
{
    Scene::setup();
	
    mSplineCam.setup();

	// load texture
	try { mTexture = gl::Texture( loadImage( loadResource("spectrum.png") ) ); }
	catch( const std::exception &e ) { console() << e.what() << std::endl; }

	// create VBO mesh
	gl::VboMesh::Layout layout;
	layout.setDynamicPositions();
	layout.setStaticIndices();
	layout.setStaticTexCoords2d();

	mVboMesh = gl::VboMesh( TRAIL_LENGTH, TRAIL_LENGTH, layout, GL_TRIANGLE_STRIP );

	// observation: indices and texture coordinates never change
	std::vector< uint32_t >	indices;	indices.reserve( TRAIL_LENGTH );
	std::vector< Vec2f >	texcoords;	texcoords.reserve( TRAIL_LENGTH );
	for( size_t i=0; i<TRAIL_LENGTH; ++i ) {
		indices.push_back( i );

		float x = math<float>::floor( i * 0.5f ) / ( TRAIL_LENGTH * 0.5f );
		float y = float( i % 2 );
		texcoords.push_back( Vec2f( x, y ) );
	}

	// create index and texture coordinate buffers
	mVboMesh.bufferIndices( indices );
	mVboMesh.bufferTexCoords2d( 0, texcoords );
	mVboMesh.unbindBuffers();

	// clear our trail buffer
	mTrail.clear();
	
	// initialize time and angle
	mTime = getElapsedSeconds();
	mAngle= 0.0f;
    
    mTrailsPerSecond = 2000.0f;
    mPhiScale = 0.01f;
    mPhiOffset = 0.01f;
    mThetaScale = 0.03f;
    //mThetaOffset = 0.03f;
    mRadius = 45.0f;
    mTwist = 20.0f;
    mWidth = 1.0f;
    mAngleIncrement = 1.0f;
    mWireframe = false;
    
    mCamType = CAM_SPLINE;

	// disable vertical sync, so we can see the actual frame rate
	//gl::disableVerticalSync();
}

void Trails::setupInterface()
{
    mInterface->addParam(CreateFloatParam("Trails/s", &mTrailsPerSecond)
                         .minValue(100.0f)
                         .maxValue(5000.0f)
                         .oscReceiver(mName,"trailspersec")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Phi Scale", &mPhiScale)
                         .minValue(0.0f)
                         .maxValue(0.1f)
                         .oscReceiver(mName,"phiscale")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Phi Offset", &mPhiOffset)
                         .minValue(0.0f)
                         .maxValue(0.1f)
                         .oscReceiver(mName,"phioffset")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Theta Scale", &mThetaScale)
                         .minValue(0.0f)
                         .maxValue(0.1f)
                         .oscReceiver(mName,"thetascale")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Radius", &mRadius)
                         .minValue(1.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName,"radius")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Twist", &mTwist)
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName,"twist")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Width", &mWidth)
                         .minValue(0.0f)
                         .maxValue(5.0f)
                         .oscReceiver(mName,"width")
                         .sendFeedback());
    mInterface->addParam(CreateFloatParam("Angle Incr", &mAngleIncrement)
                         .minValue(0.0f)
                         .maxValue(10.0f)
                         .oscReceiver(mName,"angleincr")
                         .sendFeedback());
    mInterface->addParam(CreateBoolParam("Wireframe", &mWireframe)
                         .oscReceiver(mName,"wireframe"));
    
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical());
    mSplineCam.setupInterface(mInterface, mName);
    
}

//void Trails::setupDebugInterface()
//{
//    Scene::setupDebugInterface(); // add all interface params
//    
//    mDebugParams.setOptions("Trails/s", "min=0 max=10000");
//}

void Trails::update(double dt)
{
	// find out how many trails we should add
	double			elapsed = getElapsedSeconds() - mTime;
	uint32_t		num_trails = uint32_t(elapsed * mTrailsPerSecond);
	
	// add this number of trails 
	// (note: it's an ugly function that draws a swirling trail around a sphere, just for demo purposes)
	for(size_t i=0; i<num_trails; ++i ) {
		float		phi = mAngle * mPhiScale;
		float		prev_phi = phi - mPhiOffset;
		float		theta = phi * mThetaScale;
		float		prev_theta = prev_phi * mThetaScale;

		Vec3f		pos = mRadius * Vec3f( sinf( phi ) * cosf( theta ), sinf( phi ) * sinf( theta ), cosf( phi ) );
		Vec3f		prev_pos = mRadius * Vec3f( sinf( prev_phi ) * cosf( prev_theta ), sinf( prev_phi ) * sinf( prev_theta ), cosf( prev_phi ) );

		Vec3f		direction = pos - prev_pos;
		Vec3f		right = Vec3f( sinf( mTwist * phi ), 0.0f, cosf( mTwist * phi ) );
		Vec3f		normal = direction.cross( right ).normalized();

		// add two vertices, one at each side of the center line
		mTrail.push_front( pos - mWidth * normal );
		mTrail.push_front( pos + mWidth * normal );

		mAngle += mAngleIncrement;
	}

	// keep trail length within bounds
	while( mTrail.size() > TRAIL_LENGTH )
		mTrail.pop_back();

	// copy to trail to vbo (there's probably a faster way than this, need to check that out later)
	gl::VboMesh::VertexIter itr = mVboMesh.mapVertexBuffer();
	for( size_t i=0; i<mTrail.size(); ++i, ++itr )
		itr.setPosition( mTrail[i] );

	// advance time
	mTime += num_trails / mTrailsPerSecond;
    
    if( mCamType == CAM_SPLINE )
        mSplineCam.update(dt);
}

void Trails::draw()
{
	// clear window
	//gl::clear();

	// set render states
    if( mWireframe )
        gl::enableWireframe();
    
	gl::enableAlphaBlending();

	gl::enableDepthRead();
	gl::enableDepthWrite();
    
    gl::enable( GL_TEXTURE_2D );
	gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    
    //CameraPersp cam( getWindowWidth(), getWindowHeight(), 60.0f, 0.1f, 500.0f );
    //cam.setEyePoint( Vec3f(0, 0, -100.0f) );
    //cam.setCenterOfInterestPoint( Vec3f::zero() );

	// enable 3D camera
	gl::pushMatrices();
	gl::setMatrices( getCamera() );

	// draw VBO mesh using texture
	mTexture.enableAndBind();
	gl::drawRange( mVboMesh, 0, mTrail.size(), 0, mTrail.size()-1 );
	mTexture.unbind();

	// restore camera and render states
    gl::disable( GL_TEXTURE_2D );
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::disableAlphaBlending();
    
    if( mWireframe )
        gl::disableWireframe();
}

const Camera& Trails::getCamera()
{
    switch( mCamType )
    {
        case CAM_SPLINE:
            return mSplineCam.getCamera();
            
        case CAM_CATALOG:
        {
            Scene* scene = mApp->getScene("catalog");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mSplineCam.getCamera();
            }
        }
            
        case CAM_ORBITER:
        {
            Scene* scene = mApp->getScene("orbiter");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mSplineCam.getCamera();
            }
        }
            
        default:
            return mApp->getMayaCam();
    }
}
