//
// Polyhedron.cpp
// Oculon
//
// Created by Ehsan on 12-01-05.
// Copyright 2012 ewerx. All rights reserved.
//



#include "Constants.h"
#include "Interface.h"
#include "MeshHelper.h"
#include "OculonApp.h"
#include "Polyhedron.h"
#include "Resources.h"
#include "Utils.h"
#include "cinder/CinderMath.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace std;


// ----------------------------------------------------------------
//
Polyhedron::Polyhedron()
: Scene("polyhedron")
{
}

// ----------------------------------------------------------------
//
Polyhedron::~Polyhedron()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::setup()
{
    Scene::setup();
    
    //loadMesh();
    
    createMeshes();
    
    reset();
}

void Polyhedron::loadMesh()
{
    //  ObjLoader loader( (DataSourceRef)loadResource( RES_CUBE_OBJ ) );
    //	loader.load( &mMesh );
    //	mVboMesh = gl::VboMesh( mMesh );
}

void Polyhedron::createMeshes()
{
    mVboMesh = gl::VboMesh( MeshHelper::createSphere( Vec2i(64,64) ) );
}

// ----------------------------------------------------------------
//
void Polyhedron::setupInterface()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::setupDebugInterface()
{
    Scene::setupDebugInterface();
}

// ----------------------------------------------------------------
//
void Polyhedron::reset()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::resize()
{
}

// ----------------------------------------------------------------
//
void Polyhedron::update(double dt)
{
}

// ----------------------------------------------------------------
//
void Polyhedron::draw()
{
    // setup
    if ( mLightEnabled ) {
		gl::enable( GL_LIGHTING );
	}
	if ( mTextureEnabled && mTexture ) {
		gl::enable( GL_TEXTURE_2D );
		mTexture.bind();
	}
	if ( mWireframe ) {
		gl::enableWireframe();
	}
    
    // draw
    gl::pushMatrices();
    gl::setMatrices(mApp->getMayaCam());
    
    gl::color( Color::white() );
    
    gl::draw( mVboMesh );
    
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
