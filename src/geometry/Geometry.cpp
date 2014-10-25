//
//  Geometry.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-04.
//
//

#include "Geometry.h"
#include "OculonApp.h"
#include "Interface.h"
#include "Pentagon.h"

#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;


#pragma mark - Construction

Geometry::Geometry()
: Scene("geometry")
{
    
}

Geometry::~Geometry()
{
}

#pragma mark - Setup

void Geometry::setup()
{
    Scene::setup();
    
    // params
    mDrawWireframe = true;
    
    // camera
    mCameraController.setup(mApp);
    mApp->setCamera(Vec3f(480.0f, 0.0f, 0.0f), Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f,1.0f,0.0f));
    
    // objects
    mObjects.push_back( new Pentagon(this, 100.0f) );
}

void Geometry::reset()
{
    
}

void Geometry::shutdown()
{
    for(auto object : mObjects)
    {
        delete object;
    }
    mObjects.clear();
}

#pragma mark - Interface

void Geometry::setupInterface()
{
    mInterface->addParam(CreateBoolParam( "wireframe", &mDrawWireframe )
                         .oscReceiver(getName()));
}

#pragma mark - Update

void Geometry::update(double dt)
{
    for (Entityf* entity : mObjects)
    {
        entity->update(dt);
    }
    
    Scene::update(dt);
}

#pragma mark - Draw

const Camera& Geometry::getCamera()
{
    return mCameraController.getCamera();
}

void Geometry::draw()
{
    glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT );
    gl::pushMatrices();
    
    gl::setViewport( mApp->getViewportBounds() );
	gl::setMatrices( getCamera() );
    
    if ( mDrawWireframe )
    {
		gl::enableWireframe();
	}
    
    for (Entityf* entity : mObjects)
    {
        entity->draw();
    }
    
    if ( mDrawWireframe )
    {
		gl::disableWireframe();
	}
    
    gl::popMatrices();
    glPopAttrib();
}
