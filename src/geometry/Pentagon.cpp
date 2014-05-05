//
//  Pentagon.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-04.
//
//

#include "Pentagon.h"
#include "MeshHelper.h"

using namespace ci;

Pentagon::Pentagon(Scene* scene, float radius)
: Entityf(scene)
, mRadius(radius)
{
    setup();
}

void Pentagon::setup()
{
    mMesh = gl::VboMesh( MeshHelper::createSphere( Vec2i(5,2) ) );
}

void Pentagon::update(double dt)
{
    
}

void Pentagon::draw()
{
    gl::pushMatrices();
    
    gl::scale(mRadius,mRadius,mRadius);
    gl::draw( mMesh );
    
    gl::popMatrices();
}