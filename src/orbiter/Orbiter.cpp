//
//  Orbiter.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-10-25.
//
//

#include "Orbiter.h"
#include "OrbiterSystem.h"
#include "OculonApp.h"
#include "Utils.h"
#include "SplineCam.h"
#include "SpinCam.h"
#include "OtherSceneCam.h"
#include "cinder/ImageIo.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

GLfloat Orbiter::no_mat[]			= { 0.0f, 0.0, 0.0f, 1.0f };
GLfloat Orbiter::mat_ambient[]		= { 0.5f, 0.5, 0.5f, 1.0f };
GLfloat Orbiter::mat_diffuse[]		= { 0.8f, 0.8, 0.8f, 1.0f };
GLfloat Orbiter::mat_specular[]     = { 1.0f, 1.0, 1.0f, 1.0f };
GLfloat Orbiter::mat_emission[]     = { 0.15f, 0.15f, 0.15f, 0.0f };

GLfloat Orbiter::mat_shininess[]	= { 128.0f };
GLfloat Orbiter::no_shininess[]     = { 0.0f };


#pragma mark - Construction

Orbiter::Orbiter()
: Scene("orbiter")
{
    mEnableFrustumCulling = true;
    mEnableDepthBuffer = true;
}

Orbiter::~Orbiter()
{
}

#pragma mark - Setup

void Orbiter::setup()
{
    Scene::setup();
    
    //mBodyShader = Utils::loadVertAndFragShaders("orbiter_body_vert.glsl", "orbiter_body_frag.glsl");
    mBodyShader = Utils::loadFragShader("orbiter_body_frag.glsl");
    
    // audio
    mAudioInputHandler.setup(true);
    
    mTimeController.setTimeScale(0.01f);
    
    mCameraController.setup(mApp);
    mCameraController.addCamera( new SpinCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new SplineCam(mApp->getViewportAspectRatio()) );
    mCameraController.addCamera( new OtherSceneCam(mApp, "graviton") );
    mCameraController.addCamera( new OtherSceneCam(mApp, "parsec") );
    mCameraController.setCamIndex(1);
    
    // default system
    SystemRef sol( new Orbiter::System("sol") );
    
    Vec3f pos = Vec3f::zero();
    Vec3f vel = Vec3f::zero();
    float radius = 0.0f;
    float mass = 0.0f;
    float rotationSpeed = 0.0f;
    float orbitalRadius = 0.0f;
    float offsetAngle = 0.0f;

    // fake numbers!
    const float EARTH_ORBITAL_RADIUS = 300.0f;
    const float EARTH_RADIUS = 30.0f;
    const float EARTH_MASS = 1.0f;
    const float EARTH_ROTATION_SPEED = 1.0f; // 29.78 km/s
    const float EARTH_ORBIT_SPEED = 5.0f;
    
    BodyRef body;
    
    // sun
    mass = 333000.0f;
    radius = 20;
    orbitalRadius = 0.0f;
    rotationSpeed = 0.0f;
    body = BodyRef( new Orbiter::Body("sol",
                                      pos,
                                      vel,
                                      radius,
                                      rotationSpeed,
                                      mass) );
    body->setTextures(loadImage(loadResource("sunmap.jpg")),
                      loadImage(loadResource("venus_bump.png")) );
    sol->addBody( body );
    
    // mercury
    mass = 0.055f * EARTH_MASS;
    radius = 0.3829f * EARTH_RADIUS;
    orbitalRadius = 0.4f * EARTH_ORBITAL_RADIUS;
    rotationSpeed = 1.240E-06 * EARTH_ROTATION_SPEED;
    offsetAngle = toRadians(Rand::randFloat(3.0f, 8.0f));
    pos.y = orbitalRadius * sin( offsetAngle );
    pos.z = orbitalRadius * cos( offsetAngle );
    vel.x = 1.59f * EARTH_ORBIT_SPEED;
    
    body = BodyRef( new Orbiter::Body("mercury",
                                      pos,
                                      vel,
                                      radius,
                                      rotationSpeed,
                                      mass) );
    body->setTextures(loadImage(loadResource("mercury_diffuse.jpg")),
                      loadImage(loadResource("venus_bump.png")) );
    sol->addBody( body );
    
    // venus
    mass = 0.815f * EARTH_MASS;
    radius = 0.9499f * EARTH_RADIUS;
    orbitalRadius = 0.7f * EARTH_ORBITAL_RADIUS;
    rotationSpeed = -2.99E-07 * EARTH_ROTATION_SPEED;
    offsetAngle = toRadians(Rand::randFloat(3.0f, 8.0f));
    pos.y = orbitalRadius * sin( offsetAngle );
    pos.z = orbitalRadius * cos( offsetAngle );
    vel.x = 1.175f * EARTH_ORBIT_SPEED;
    
    body = BodyRef( new Orbiter::Body("venus",
                                      pos,
                                      vel,
                                      radius,
                                      rotationSpeed,
                                      mass) );
    body->setTextures(loadImage(loadResource("venus_diffuse.jpg")),
                      loadImage(loadResource("venus_bump.png")) );
    sol->addBody( body );
    
    // earth
    mass = 1.0f * EARTH_MASS;
    radius = 1.0f * EARTH_RADIUS;
    orbitalRadius = 1.0f * EARTH_ORBITAL_RADIUS;
    rotationSpeed = 7.30E-05 * EARTH_ROTATION_SPEED;
    offsetAngle = toRadians(Rand::randFloat(3.0f, 8.0f));
    pos.y = orbitalRadius * sin( offsetAngle );
    pos.z = orbitalRadius * cos( offsetAngle );
    vel.x = 1.0f * EARTH_ORBIT_SPEED;
    
    body = BodyRef( new Orbiter::Body("earth",
                                      pos,
                                      vel,
                                      radius,
                                      rotationSpeed,
                                      mass) );
    body->setTextures(loadImage(loadResource("earth_diffuse.png")),
                      loadImage(loadResource("earth_bump.png")) );
    sol->addBody( body );
    
    // mars
    mass = 0.107f * EARTH_MASS;
    radius = 0.533f * EARTH_RADIUS;
    orbitalRadius = 1.5f * EARTH_ORBITAL_RADIUS;
    rotationSpeed = 7.09E-05 * EARTH_ROTATION_SPEED;
    offsetAngle = toRadians(Rand::randFloat(3.0f, 8.0f));
    pos.y = orbitalRadius * sin( offsetAngle );
    pos.z = orbitalRadius * cos( offsetAngle );
    vel.x = 0.808f * EARTH_ORBIT_SPEED;
    
    body = BodyRef( new Orbiter::Body("mars",
                                      pos,
                                      vel,
                                      radius,
                                      rotationSpeed,
                                      mass) );
    body->setTextures(loadImage(loadResource("mars_diffuse.png")),
                      loadImage(loadResource("mars_bump.png")) );
    sol->addBody( body );
    
    mSystems.push_back(sol);
}

void Orbiter::reset()
{
    
}

#pragma mark - Interface

void Orbiter::setupInterface()
{
    mTimeController.setupInterface(mInterface, getName());
    
    mInterface->gui()->addColumn();
    
    mCameraController.setupInterface(mInterface, getName());
    mAudioInputHandler.setupInterface(mInterface, getName());
}

#pragma mark - Update

void Orbiter::update(double dt)
{
    mTimeController.update(dt);
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    mCameraController.update(dt);
    
    mSystems[mCurrentSystemIndex]->update(dt);
    
    Scene::update(dt);
}

#pragma mark - Draw

const Camera& Orbiter::getCamera()
{
    return mCameraController.getCamera();
}

void Orbiter::draw()
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    
    glShadeModel( GL_SMOOTH );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_NORMALIZE );
    gl::enableAlphaBlending();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    glClear( GL_DEPTH_BUFFER_BIT );
    
    glDepthFunc( GL_LESS );
    glEnable( GL_DEPTH_TEST );
    
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );
    
    glEnable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    
    gl::pushMatrices();
    gl::setViewport( mApp->getViewportBounds() );
    gl::setMatrices( getCamera() );
    
    // lighting
    {
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0 );
        
        GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glLightfv( GL_LIGHT0, GL_POSITION, light_position );
        glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f );
        glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f );
        glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f );//0.00015f );
        
        ColorA color( 0.95f, 0.95f, 0.95f, 1.0f );
        glMaterialfv( GL_FRONT, GL_DIFFUSE, color );
        glMaterialfv( GL_FRONT, GL_AMBIENT,	Orbiter::no_mat );
        glMaterialfv( GL_FRONT, GL_SPECULAR, Orbiter::no_mat );
        glMaterialfv( GL_FRONT, GL_SHININESS, Orbiter::no_shininess );
        glMaterialfv( GL_FRONT, GL_EMISSION, Orbiter::mat_emission );
    }
    
    mBodyShader.bind();
    mBodyShader.uniform( "texDiffuse", 0 );
    mBodyShader.uniform( "texNormal", 1 );
//    mBodyShader.uniform( "texMask", 2 );
    mBodyShader.uniform( "lightDir", Vec3f(0.0f,0.0f,0.0f) );
    
    mSystems[mCurrentSystemIndex]->draw();
    
    mBodyShader.unbind();
    
    gl::popMatrices();
    glPopAttrib();
}

void Orbiter::drawDebug()
{
    Scene::drawDebug();
}
