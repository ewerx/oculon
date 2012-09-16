/*
 *  Polyhedron.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Polyhedron.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Constants.h"
#include "Resources.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace std;


//
// Polyhedron
// 

Polyhedron::Polyhedron(Scene* scene)
: SubScene(scene)
{
}

Polyhedron::~Polyhedron()
{
}

void Polyhedron::setup()
{
    ObjLoader loader( (DataSourceRef)loadResource( RES_CUBE_OBJ ) );
	loader.load( &mMesh );
	mVBO = gl::VboMesh( mMesh );
    
    reset();
}

void Polyhedron::setupInterface()
{
    Interface* interface = mParentScene->getInterface();
    const string name("polyhedron");
    interface->gui()->addColumn();
    interface->gui()->addLabel(name);
    
}

void Polyhedron::setupDebugInterface()
{
}

void Polyhedron::reset()
{
}

void Polyhedron::resize()
{
}

void Polyhedron::update(double dt)
{
}

void Polyhedron::draw()
{
    gl::pushMatrices();
    
    gl::color( Color::white() );
    gl::draw( mVBO );
    
    gl::popMatrices();

}
