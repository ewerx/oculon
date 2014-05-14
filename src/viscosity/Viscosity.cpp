//
//  Viscosity.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-11.
//
//

#include "OculonApp.h"
#include "Viscosity.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma mark - Construction

Viscosity::Viscosity()
: Scene("viscosity")
{
}

Viscosity::~Viscosity()
{
}

#pragma mark - Setup

void Viscosity::setup()
{
    Scene::setup();
    
    // params
    mVelocityScale = 0.5f;
    mDensityScale = 0.5f;
    mColor = ColorAf::white();
    mDrawMesh = false;

    mCamera.setOrtho( 0, mApp->getViewportWidth(), mApp->getViewportHeight(), 0, -1, 1 );
    
    mFluid2D.set( mApp->getViewportWidth()/3.0f, mApp->getViewportHeight()/3.0f );
   	mFluid2D.setDensityDissipation( 0.99f );
	mFluid2D.setRgbDissipation( 0.99f );
	mFluid2D.enableDensity();
	mFluid2D.enableRgb();
	mFluid2D.enableVorticityConfinement();
	mFluid2D.initSimData();
    
    // setup for mesh mode
    mFluid2D.enableTexCoord();
	mFluid2D.setTexCoordViscosity( 1.0f );
    
    //mTex = gl::Texture( loadImage( loadResource( "earthGray.png" ) ) );
    
    // Points and texture coordinates
	for( int j = 0; j < mFluid2D.resY(); ++j )
    {
		for( int i = 0; i < mFluid2D.resX(); ++i )
        {
			mTriMesh.appendVertex( Vec2f( 0.0f, 0.0f ) );
			mTriMesh.appendTexCoord( Vec2f( 0.0f, 0.0f ) );
		}
	}
	// Triangles
	for( int j = 0; j < mFluid2D.resY() - 1; ++j )
    {
		for( int i = 0; i < mFluid2D.resX() - 1; ++i )
        {
			int idx0 = (j + 0)*mFluid2D.resX() + (i + 0 );
			int idx1 = (j + 1)*mFluid2D.resX() + (i + 0 );
			int idx2 = (j + 1)*mFluid2D.resX() + (i + 1 );
			int idx3 = (j + 0)*mFluid2D.resX() + (i + 1 );
			mTriMesh.appendTriangle( idx0, idx1, idx2 );
			mTriMesh.appendTriangle( idx0, idx2, idx3 );
		}
	}
    
    mAudioInputHandler.setup();
    
    mSplatNodeController.addFormation( new MirrorBounceFormation(mApp->getViewportWidth() * 0.75f) );
}

void Viscosity::reset()
{
}

#pragma mark - Interface

void Viscosity::setupInterface()
{
    mInterface->addParam(CreateColorParam("color", &mColor, kMinColor, kMaxColor)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("velocity_scale", &mVelocityScale)
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("density_scale", &mDensityScale)
                         .oscReceiver(mName));
    
    mInterface->addParam(CreateBoolParam("buoyancy", mFluid2D.enableBuoyancyAddr())
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("temperature", mFluid2D.ambientTemperatureAddr())
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("buoyancy_amt", mFluid2D.materialBuoyancyAddr())
                         .oscReceiver(mName));
    mInterface->addParam(CreateFloatParam("material_wt", mFluid2D.materialWeightAddr())
                         .oscReceiver(mName));
    
    
    mSplatNodeController.setupInterface(mInterface, mName);
    mAudioInputHandler.setupInterface(mInterface, mName);
}

void Viscosity::handleMouseDown(const ci::app::MouseEvent &event)
{
    mPrevPos = event.getPos();
}

void Viscosity::handleMouseDrag(const ci::app::MouseEvent &event)
{
    float x = ((float)event.getX() / mApp->getViewportWidth()) * mFluid2D.resX();//Rand::randFloat() * mFluid2D.resX();
	float y = ((float)event.getY() / mApp->getViewportHeight()) * mFluid2D.resY();//Rand::randFloat() * mFluid2D.resY();
	
    //float mVelScale = 600.0f;
    float mRgbScale = 50.0f;
    //float mDenScale = 50.0f;
    //Colorf mColor = Colorf::white();
    
	if( event.isLeftDown() )
    {
		Vec2f dv = event.getPos() - mPrevPos;
		mFluid2D.splatVelocity( x, y, mVelocityScale * 10000.0f * dv );
		mFluid2D.splatRgb( x, y, mRgbScale*mColor );
		if( mFluid2D.isBuoyancyEnabled() )
        {
			mFluid2D.splatDensity( x, y, mDensityScale * 1000.0f );
		}
        
        //console() << "splat: " << x << ", " << y << std::endl;
	}
	
	mPrevPos = event.getPos();
}

#pragma mark - Update

void Viscosity::update(double dt)
{
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mSplatNodeController.update(dt, mAudioInputHandler);
    
    for ( NodeFormation::Node& node : mSplatNodeController.getNodes() )
    {
        if (abs(node.mPosition.x) < 0.001f && abs(node.mPosition.y) < 0.001f)
        {
            //continue;
        }
        
        float x = (node.mPosition.x * 0.5f + 0.5f) * mFluid2D.resX();
        float y = (node.mPosition.y * 0.5f + 0.5f) * mFluid2D.resY();
        
        mFluid2D.splatVelocity( x, y, node.mVelocity.xy() * mVelocityScale * 10000.0f );
        mFluid2D.splatRgb( x, y, 50.0f*mColor );
        if( mFluid2D.isBuoyancyEnabled() )
        {
			mFluid2D.splatDensity( x, y, mDensityScale * 1000.0f );
		}
        
        //console() << "splat: " << x << ", " << y << std::endl;
    }
    
    mFluid2D.step();
}

#pragma mark - Draw

const Camera& Viscosity::getCamera()
{
    return mCamera;
}

void Viscosity::draw()
{
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    
    if (mDrawMesh)
    {
        // Update the positions and tex coords
        Rectf drawRect = mApp->getViewportBounds();
        int limX = mFluid2D.resX() - 1;
        int limY = mFluid2D.resY() - 1;
        float dx = drawRect.getWidth()/(float)limX;
        float dy = drawRect.getHeight()/(float)limY;
        
        for( int j = 0; j < mFluid2D.resY(); ++j ) {
            for( int i = 0; i < mFluid2D.resX(); ++i ) {
                Vec2f P = Vec2f( i*dx, j*dy );
                Vec2f uv = mFluid2D.texCoordAt( i, j );
                
                int idx = j*mFluid2D.resX() + i;
                mTriMesh.getVertices()[idx] = P;
                mTriMesh.getTexCoords()[idx] = uv;
                
            }
        }
        
        mTex.bind();
        gl::draw( mTriMesh );
        mTex.unbind();
    }
    else
    {
        float* data = const_cast<float*>( (float*)mFluid2D.rgb().data() );
        Surface32f surf( data, mFluid2D.resX(), mFluid2D.resY(), mFluid2D.resX()*sizeof(Colorf), SurfaceChannelOrder::RGB );
        
        if ( ! mTex )
        {
            mTex = gl::Texture( surf );
        }
        else
        {
            mTex.update( surf );
        }
        gl::draw( mTex, mApp->getViewportBounds() );
        mTex.unbind();
    }
    
    //mSplatNodeController.draw();
    
    gl::popMatrices();
}

void Viscosity::drawDebug()
{
    Scene::drawDebug();
    
    gl::pushMatrices();
    gl::setMatricesWindow( mApp->getViewportSize() );
    
    //mSplatNodeController.drawDebug();
    gl::color(1.0f, 0.0f, 0.0f);
    for ( NodeFormation::Node& node : mSplatNodeController.getNodes() )
    {
        Vec2f pos = node.mPosition.xy();
        pos *= Vec2f(0.5f,0.5f);
        pos += Vec2f(0.5f,0.5f);
        pos *= mApp->getViewportSize();
        gl::drawSolidCircle(pos, 10.0f);
    }
    
    gl::popMatrices();
}
