/*
 *  Orbiter.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-19.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

//#include <boost/format.hpp>
#include "AudioInput.h" // compile errors if this is not before App.h
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"
#include "Orbiter.h"
#include "Sun.h"

using namespace ci;

GLfloat Orbiter::no_mat[]			= { 0.0, 0.0, 0.0, 1.0 };
GLfloat Orbiter::mat_ambient[]		= { 0.4, 0.1, 0.2, 1.0 };//{ 0.6, 0.3, 0.4, 1.0 };
GLfloat Orbiter::mat_diffuse[]		= { 0.3, 0.5, 0.8, 1.0 };
GLfloat Orbiter::mat_specular[]		= { 1.0, 1.0, 1.0, 1.0 };
GLfloat Orbiter::mat_emission[]		= { 0.0, 0.1, 0.3, 0.0 };

GLfloat Orbiter::mat_shininess[]	= { 128.0 };
GLfloat Orbiter::no_shininess[]		= { 0.0 };

//TODO: read from data file
#define PLANETS_TUPLE \
PLANETS_ENTRY("Mercury",57900000000.f,  2440000,3.33E+023,  47900 ) \
PLANETS_ENTRY("Venus",  108000000000.f, 6050000,4.869E+024, 35000 ) \
PLANETS_ENTRY("Earth",  150000000000.f, 6378140,5.976E+024, 29800 ) \
PLANETS_ENTRY("Mars",   227940000000.f, 3397200,6.421E+023, 24100 ) \
PLANETS_ENTRY("Jupiter",778330000000.f, 71492000,1.9E+027,  13100 ) \
PLANETS_ENTRY("Saturn", 1429400000000.f,60268000,5.688E+026,    9640 ) \
PLANETS_ENTRY("Uranus", 2870990000000.f,25559000,8.686E+025,    6810 ) \
PLANETS_ENTRY("Neptune",4504300000000.f,24746000,1.024E+026,    5430 ) \
PLANETS_ENTRY("Pluto",  5913520000000.f,1137000,1.27E+022,  4740 )
//end tuple

/*static*/ double Orbiter::sDefaultTimeScale = 60.f * 60.f * 24.f * 10.f; // 10 days
/*static*/ double Orbiter::sDefaultGravityConstant = 6.6742e-11;

//
// Orbiter
//
Orbiter::Orbiter()
: Scene()
, mSun()
, mFollowTarget()
, mIsFollowCameraEnabled(false)
{
    mEnableFrustumCulling = true;
}

Orbiter::~Orbiter()
{
    // midi map will get destroyed
}

void Orbiter::setup()
{
    mTimeScale = sDefaultTimeScale;
    mDrawScale = 6e-12;
    mGravityConstant = sDefaultGravityConstant;
    
    mLightDirection = Vec3f( 0.025f, 0.25f, 1.0f );
	mLightDirection.normalize();
    
    mMidiMap.init(&mApp->getMidiInput());
    setupMidiMapping();
    
    reset();
}

void Orbiter::setupMidiMapping()
{
    // setup MIDI inputs for learning
    mMidiMap.registerMidiEvent("orb_gravity", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    mMidiMap.registerMidiEvent("orb_timescale", MidiEvent::TYPE_VALUE_CHANGE, this, &Orbiter::handleGravityChange);
    mMidiMap.beginLearning();
    // ... or load a MIDI mapping
    //mMidiInput.setMidiKey("gravity", channel, note);
}

void Orbiter::setupParams(params::InterfaceGl& params)
{
    params.addText( "text", "label=`Orbiter`" );
    params.addParam("Gravity Constant", &mGravityConstant, "step=0.00000000001");
    params.addParam("Time Scale", &mTimeScale, "step=100.0");
    params.addSeparator();
    params.addParam("Frustum Culling", &mEnableFrustumCulling, "keyIncr=f");
}

void Orbiter::reset()
{
    mElapsedTime = 0.0f;
    
    const float radialEnhancement = 1000000.0f;
    mBodies.clear(); // this will delete mSun
    
    // sun
    double mass = 1.989E+030;
    float radius = 35.0f;
    double orbitalRadius;
    double orbitalVel;
    mSun = new Sun(Vec3d::zero(),
                   Vec3d::zero(),
                   radius,
                   mass,
                   ColorA(1.0f, 1.0f, 1.0f));
    mSun->setup();
    mSun->setLabelVisible(false);
    mBodies.push_back(mSun);
    
    Body* body;
    
#define PLANETS_ENTRY(name,orad,brad,mss,ovel) \
    mass = mss;\
    orbitalRadius = orad;\
    orbitalVel = ovel;\
    radius = brad * mDrawScale * radialEnhancement;\
    body = new Body(Vec3d(-orbitalRadius, 0.0f, 0.0f), \
                    Vec3d(0.0f, orbitalVel, 0.0f),\
                    radius, \
                    mass, \
                    ColorA(0.3f, 0.5f, 0.7f)); \
    body->setup(); \
    mBodies.push_back( body );
    PLANETS_TUPLE
#undef PLANETS_ENTRY
    
    /*
    
    
    // venus
    mass = 4.869E+024; 
    double orbitalRadius = 108000000000.f;
    double orbitalVel = 35000;
    radius = 20.0f;
    mBodies.push_back(Body(Vec3d(-orbitalRadius, 0.0f, 0.0f), 
                           Vec3d(0.0f, orbitalVel, 0.0f),
                           radius, 
                           mass, 
                           ColorA(0.3f, 0.5f, 0.7f)) ); 
    
    // earth
    mass = 5.976E+024;
    orbitalRadius = 150000000000.f;
    orbitalVel = 29800;
    mBodies.push_back(Body(Vec3d(-orbitalRadius, 0.0f, 0.0f), 
                           Vec3d(0.0f, orbitalVel, 0.0f),
                           radius, 
                           mass, 
                           ColorA(0.1f, 0.8f, 0.3f)) );
     
     */
    
    // random comets
    int num_comets = 4;
    //Vec3f orbitalPlaneN = Vec3d( Rand::randFloat(
    
    for( int i = 0; i < num_comets; ++i )
    {
        mass = 1e9;
        radius = 10.0f;
        double angle = Rand::randFloat(2*M_PI);
        orbitalRadius = (Rand::randInt(100000) + 100000 ) * 4e6;
        Vec3d pos( orbitalRadius * sin ( angle ), 0.0f, orbitalRadius * cos ( angle ) );
        //Vec3d orbitalPlaneNormal = pos.normalized();
        orbitalVel = ( ( rand() % 200 ) + 100 ) * 50.0;
        
        mBodies.push_back(new Body(pos, 
                                   /*orbitalPlaneNormal * orbitalVel,*/
                                   Vec3d(0.0f, orbitalVel, 0.0f),
                                   radius, 
                                   mass, 
                                   ColorA(0.1f, 0.8f, 0.3f)) );
    }
    
    mFollowTarget = mBodies.back();
    // TODO: determine orbital velocity
    /*
     for( int i = 1; i < mBodies.size(); ++i )
     {
     Vec3d dirToCenter = mBodies[0].mPosition - mBodies[i].mPosition;
     Vec3d up(0.f, 0.f, 1.f);
     Vec3d initVelDir = dirToCenter.cross(up);
     initVelDir.normalize();
     float magnitude = mEscapeVelocity * (Orbiter::sGravityConstant * mBodies[0].mMass) / dirToCenter.length();
     //float magnitude =  
     mBodies[i].mVelocity = initVelDir * magnitude;
     }
     */
    
    AudioInput& audioInput = mApp->getAudioInput();
    audioInput.setFftBandCount(mBodies.size());
}

void Orbiter::update(double dt)
{
    dt *= mTimeScale;
    mElapsedTime += dt;
    
    bool simulate = true;
    bool symmetric = true;
    int i = 0;
    bool print = false;
    
    if(print)
        console() << "dt: " << dt << std::endl;
    
    if( simulate )
    {
        for(BodyList::iterator bodyIt = mBodies.begin(); 
            bodyIt != mBodies.end();
            ++bodyIt)
        {   
            Body* body = (*bodyIt);
            if( symmetric )
            {
                // N-bodies
                for(BodyList::iterator otherBody = mBodies.begin();
                    otherBody != mBodies.end();
                    ++otherBody)
                {
                    if( bodyIt != otherBody )
                        body->applyForceFromBody(*(*otherBody),dt,mGravityConstant);
                }
            }
            else
            {
                // assume sun at index 0
                if( i!=0 )
                {
                    body->applyForceFromBody(*mBodies[0], dt, mGravityConstant);
                }
            }
            
            body->update(dt);
            
            ++i;
        }
    }
    
    updateAudioResponse();
    //updateTimeDisplay();
    
    Scene::update(dt);
}

//
// handleKeyDown
//
bool Orbiter::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getChar()) 
    {
        case ' ':
            reset();
            break;
        case 'c':
            mIsFollowCameraEnabled = !mIsFollowCameraEnabled;
            mApp->setUseMayaCam( !mIsFollowCameraEnabled );
        default:
            handled = false;
            break;
    }
    
    return handled;
}

//
//
//
void Orbiter::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
    
    unsigned int bandCount = audioInput.getFftBandCount();
    float* fftBuffer = fftDataRef.get();
    
    if( fftBuffer )
    {
        for( int i = 0; i < ( bandCount ); i++ ) 
        {
            if( i < mBodies.size() )
            {
                float multiplier = math<float>::clamp(0.5f, (fftBuffer[i] / bandCount) * (4.0f+i), (4.0f+i));
                mBodies[i]->setRadiusMultiplier( multiplier );
            }
        }
    }
}

void Orbiter::draw()
{
    glPushMatrix();
    glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    
    //if( DIFFUSE )
    {
        ci::ColorA color( CM_HSV, 1.0f, 1.0f, 1.0f, 1.0f );
        glMaterialfv( GL_FRONT, GL_DIFFUSE,	color );
    } 
    /*
     else 
     {
     glMaterialfv( GL_FRONT, GL_DIFFUSE,	no_mat );
     }
     */
    
    //if( AMBIENT )
    glMaterialfv( GL_FRONT, GL_AMBIENT,	Orbiter::mat_ambient );
    //else
    //    glMaterialfv( GL_FRONT, GL_AMBIENT,	no_mat );
    
    //if( SPECULAR ){
    //    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
    //    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
    //} else {
    glMaterialfv( GL_FRONT, GL_SPECULAR, Orbiter::no_mat );
    glMaterialfv( GL_FRONT, GL_SHININESS, Orbiter::no_shininess );
    //}
    
    //if( EMISSIVE )
    glMaterialfv( GL_FRONT, GL_EMISSION, Orbiter::mat_emission );
    //else
        //glMaterialfv( GL_FRONT, GL_EMISSION, no_mat );
    
    Matrix44d matrix = Matrix44d::identity();
    matrix.scale(Vec3d( mDrawScale * getWindowWidth() / 2.0f, 
                       mDrawScale * getWindowHeight() / 2.0f,
                       mDrawScale * getWindowHeight() / 4.0f));
    
    int culled = 0;
    for(BodyList::iterator bodyIt = mBodies.begin(); 
        bodyIt != mBodies.end();
        ++bodyIt)
    {
        Body* body = (*bodyIt);
        if (!mEnableFrustumCulling ||
            isSphereInFrustum( (matrix * body->getPosition()), body->getRadius()*mDrawScale ) )
        {
            body->draw(matrix);
        }
        else
        {
            culled++;
        }
        
        //glPushMatrix();
        //Vec3d pos = matrix * bodyIt->getPosition();
        //glTranslated(pos.x, pos.y, pos.z);
        //glPopMatrix();
    }
    
    //mApp->console() << "culled " << culled << std::endl;
    
    if( mIsFollowCameraEnabled && mFollowTarget )
    {
        double offset = mFollowTarget->getRadius()*1.2f;
        Vec3d toCore = matrix * mFollowTarget->getPosition();
        Vec3d offsetVec = offset * mFollowTarget->getPosition().normalized();
        Vec3d up = Vec3d( sin(getElapsedSeconds()/20.f), 1.0f, 0.0f );
        toCore = toCore - offsetVec;
        Vec3f camPos = Vec3f( toCore.x, toCore.y, toCore.z );
        mApp->setCamera( camPos, Vec3f::zero(), up );
    }
    
    glPopMatrix();
    glDisable( GL_LIGHTING );
	glDisable( GL_LIGHT0 );
}

void Orbiter::updateTimeDisplay()
{

    char buf[64];
    snprintf(buf, 64, "%.0f hours", mElapsedTime / 3600.f);
    mApp->getInfoPanel().addLine( buf, Color(0.5f, 0.5f, 0.5f) );
}

void Orbiter::handleGravityChange(MidiEvent midiEvent)
{
    double delta = (midiEvent.getValueRatio() * 2.0f - 1.0f) * 8.0e-11;
    mGravityConstant = 6.6742e-11 + delta;
}

void Orbiter::handleTimeScaleChange(MidiEvent midiEvent)
{
    mTimeScale = sDefaultTimeScale * ( midiEvent.getValueRatio() * 100.f );
}