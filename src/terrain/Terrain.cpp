//
// Terrain.cpp
// Oculon
//
// Created by Ehsan on 13-Mar-25.
// Copyright 2013 ewerx. All rights reserved.
//

#include "Constants.h"
#include "Interface.h"
#include "MeshHelper.h"
#include "OculonApp.h"
#include "Terrain.h"
#include "Resources.h"
#include "Utils.h"
#include "cinder/CinderMath.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace std;


// ----------------------------------------------------------------
//
Terrain::Terrain()
: Scene("Terrain")
{
}

// ----------------------------------------------------------------
//
Terrain::~Terrain()
{
}

// ----------------------------------------------------------------
//
void Terrain::setup()
{
    Scene::setup();
    
    // Params
    mLightEnabled       = false;
    mTextureEnabled     = false;
    mWireframe          = true;
    mDrawInstances      = false;
    mAdditiveBlending   = true;
    mDynamicLight       = false;
    mObjectScale        = 10.0f;
    mDivision           = 1;
    mResolution         = 12;
    mSlices             = 20;
    mCamType            = CAM_MANUAL;
    mLineWidth          = 2.0f;
    mColor              = ColorAf::white();
    
    // Load shader
	try {
		mShader = gl::GlslProg( loadResource( RES_POLYHEDRON_SHADER_VERT ), loadResource( RES_POLYHEDRON_SHADER_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << ex.what() << endl;
	}
    
	// Load the texture map
	mTexture = gl::Texture( loadImage( loadResource( RES_POLYHEDRON_TEX1 ) ) );
    
    // Set up the light
	mLight = new gl::Light( gl::Light::DIRECTIONAL, 0 );
	mLight->setAmbient( ColorAf::black() );
	mLight->setDiffuse( ColorAf::gray( 0.6f ) );
	mLight->setDirection( Vec3f::one() );
	mLight->setPosition( Vec3f::one() * -1.0f );
	mLight->setSpecular( ColorAf::white() );
	mLight->enable();
    
    // camera
    mSplineCam.setup();
    
    //loadMesh();
    createMeshes();
    
    reset();
}

void Terrain::loadMesh()
{
    //  ObjLoader loader( (DataSourceRef)loadResource( RES_CUBE_OBJ ) );
    //	loader.load( &mMesh );
    //	mVboMesh = gl::VboMesh( mMesh );
}

void Terrain::createMeshes()
{
	mCylinder		= gl::VboMesh( MeshHelper::createCylinder( Vec2i( mResolution, mSlices ), 1.0f, 1.0f, false, false ) );
    
    
    float delta = ( 2.0f * (float)M_PI ) / (float)mResolution;
	float step	= 1.0f / (float)mSlices;
	float ud	= 1.0f / (float)mResolution;
    
	int32_t p = 0;
	for ( float phi = 0.0f; p <= mSlices; ++p, phi += step ) {
		int32_t t	= 0;
		float u		= 0.0f;
		for ( float theta = 0.0f; t < mResolution; ++t, u += ud, theta += delta ) {
            
			float radius = 1.0f;//lerp( baseRadius, topRadius, phi );
            
			Vec3f position(
                           math<float>::cos( theta ) * radius,
                           phi - 0.5f,
                           math<float>::sin( theta ) * radius
                           );
			srcPositions.push_back( position );
            
			Vec3f normal = Vec3f( position.x, 0.0f, position.z ).normalized();
			normal.y = 0.0f;
			srcNormals.push_back( normal );
            
			Vec2f texCoord( u, phi );
			srcTexCoords.push_back( texCoord );
		}
	}
    
	srcNormals.push_back( Vec3f( 0.0f, 1.0f, 0.0f ) );
	srcNormals.push_back( Vec3f( 0.0f, -1.0f, 0.0f ) );
	srcPositions.push_back( Vec3f( 0.0f, -0.5f, 0.0f ) );
	srcPositions.push_back( Vec3f( 0.0f, 0.5f, 0.0f ) );
	srcTexCoords.push_back( Vec2f( 0.0f, 0.0f ) );
	srcTexCoords.push_back( Vec2f( 0.0f, 1.0f ) );
	int32_t topCenter		= (int32_t)srcPositions.size() - 1;
	int32_t bottomCenter	= topCenter - 1;
}

// ----------------------------------------------------------------
//
void Terrain::setupInterface()
{
//    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
//                        .maxValue(CAM_COUNT)
//                        .oscReceiver(getName(), "camera")
//                        .isVertical());
    
    mInterface->addParam(CreateBoolParam( "wireframe", &mWireframe )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "lighting", &mLightEnabled )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "texture", &mTextureEnabled )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "additive", &mAdditiveBlending )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "dynamic light", &mDynamicLight )
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateFloatParam("scale", &mObjectScale)
                         .minValue(0.0f)
                         .maxValue(100.0f)
                         .oscReceiver(mName)
                         .sendFeedback());
    
    mInterface->addParam(CreateIntParam("res x", &mResolution)
                         .minValue(1)
                         .maxValue(20)
                         .oscReceiver(mName, "spacingx")
                         .sendFeedback());
    mInterface->addParam(CreateIntParam("slices", &mSlices)
                         .minValue(1)
                         .maxValue(200)
                         .oscReceiver(mName, "spacingx")
                         .sendFeedback());
    
    mInterface->addParam(CreateIntParam("line width", &mLineWidth)
                         .minValue(1)
                         .maxValue(8)
                         .oscReceiver(mName, "linewidth")
                         .sendFeedback());
    
    mInterface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color"));
    
//    mInterface->addParam(CreateIntParam("division", &mDivision)
//                         .minValue(1)
//                         .maxValue(100)
//                         .oscReceiver(mName, "division")
//                         .sendFeedback());
//
//    mInterface->addParam(CreateIntParam("resolution", &mResolution)
//                         .minValue(1)
//                         .maxValue(100)
//                         .oscReceiver(mName, "resolution")
//                         .sendFeedback());
    
    mInterface->gui()->addColumn();
    vector<string> camTypeNames;
#define TERRAIN_CAMTYPE_ENTRY( nam, enm ) \
    camTypeNames.push_back(nam);
    TERRAIN_CAMTYPE_TUPLE
#undef  TERRAIN_CAMTYPE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Cam Type", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camtype")
                        .isVertical(), camTypeNames);
    mSplineCam.setupInterface(mInterface, mName);
}

// ----------------------------------------------------------------
//
void Terrain::setupDebugInterface()
{
    Scene::setupDebugInterface();
}

// ----------------------------------------------------------------
//
void Terrain::reset()
{
    createMeshes();
}

// ----------------------------------------------------------------
//
void Terrain::resize()
{
}

// ----------------------------------------------------------------
//
void Terrain::update(double dt)
{
    Scene::update(dt);
    
    generate();
    
    if( mCamType == CAM_SPLINE )
        mSplineCam.update(dt);
    
    if (mDynamicLight)
    {
        mLight->setPosition( getCamera().getEyePoint() * 1.1f );
        mLight->setDirection( getCamera().getCenterOfInterestPoint() );
    }
    
    // Update light on every frame
	mLight->update( getCamera() );
}

void Terrain::generate()
{
	for ( int32_t p = 0; p < mSlices; ++p ) {
		for ( int32_t t = 0; t < mResolution; ++t ) {
			int32_t n = t + 1 >= mResolution ? 0 : t + 1;
            
			int32_t index0 = ( p + 0 ) * mResolution + t;
			int32_t index1 = ( p + 0 ) * mResolution + n;
			int32_t index2 = ( p + 1 ) * mResolution + t;
			int32_t index3 = ( p + 1 ) * mResolution + n;
            
			normals.push_back( srcNormals[ index0 ] );
			normals.push_back( srcNormals[ index2 ] );
			normals.push_back( srcNormals[ index1 ] );
			normals.push_back( srcNormals[ index1 ] );
			normals.push_back( srcNormals[ index2 ] );
			normals.push_back( srcNormals[ index3 ] );
            
			positions.push_back( srcPositions[ index0 ] );
			positions.push_back( srcPositions[ index2 ] );
			positions.push_back( srcPositions[ index1 ] );
			positions.push_back( srcPositions[ index1 ] );
			positions.push_back( srcPositions[ index2 ] );
			positions.push_back( srcPositions[ index3 ] );
            
			texCoords.push_back( srcTexCoords[ index0 ] );
			texCoords.push_back( srcTexCoords[ index2 ] );
			texCoords.push_back( srcTexCoords[ index1 ] );
			texCoords.push_back( srcTexCoords[ index1 ] );
			texCoords.push_back( srcTexCoords[ index2 ] );
			texCoords.push_back( srcTexCoords[ index3 ] );
		}
	}
    
	for ( uint32_t i = 0; i < positions.size(); ++i ) {
		indices.push_back( i );
	}
    
    mTriMesh = MeshHelper::create( indices, positions, normals, texCoords );
}

// ----------------------------------------------------------------
//
void Terrain::draw()
{
    // Set up OpenGL to work with default lighting
	glShadeModel( GL_SMOOTH );
	gl::enable( GL_POLYGON_SMOOTH );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    
    if (mAdditiveBlending)
    {
        gl::enableAdditiveBlending();
    }
    else
    {
        gl::enableAlphaBlending();
	}
    gl::enableDepthRead();
	gl::enableDepthWrite();
    
    // setup
    if ( mLightEnabled ) {
		gl::enable( GL_LIGHTING );
        Color diffuse = mColor * mColor.a;
        mLight->setDiffuse( diffuse );
        mLight->setSpecular( mColor );
	}
	if ( mTextureEnabled && mTexture ) {
		gl::enable( GL_TEXTURE_2D );
		mTexture.bind();
	}
	if ( mWireframe )
    {
        glLineWidth( (GLfloat)mLineWidth );
		gl::enableWireframe();
	}
    
    // draw
    gl::pushMatrices();
    gl::setMatrices( getCamera() );
    gl::scale( Vec3f::one() * mObjectScale );
    
    gl::color( mColor );
    
    gl::draw( getMesh() );
    
    gl::translate( 3.0f, 0.0f, 0.0f );
    gl::color(1.0f, 0.0f, 0.0f);
    
    gl::draw( mTriMesh );
    
    gl::popMatrices();
    
    // restore
    if ( mWireframe ) {
		gl::disableWireframe();
	}
	if ( mTextureEnabled && mTexture ) {
		mTexture.unbind();
		gl::disable( GL_TEXTURE_2D );
	}
	if ( mLightEnabled ) {
		gl::disable( GL_LIGHTING );
	}
}

const Camera& Terrain::getCamera()
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
            
        case CAM_GRAVITON:
        {
            Scene* scene = mApp->getScene("graviton");
            
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

const ci::gl::VboMesh& Terrain::getMesh()
{
    return mCylinder;
}
