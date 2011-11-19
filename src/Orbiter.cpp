/*
 *  Orbiter.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-19.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include <boost/format.hpp>
#include "AudioInput.h" // compile errors if this is not before App.h
#include "OculonApp.h"//TODO: fix this dependency
#include "cinder/Rand.h"
#include "Orbiter.h"
#include "Sun.h"
#include "TextEntity.h"

using namespace ci;
using namespace boost;

//TODO: read from data file (orbRad,    bodyRad, mass,      oVel)
#define PLANETS_TUPLE \
PLANETS_ENTRY("Mercury",57900000000.f,  2440000,3.33E+023,  47900 ) \
PLANETS_ENTRY("Venus",  108000000000.f, 6050000,4.869E+024, 35000 ) \
PLANETS_ENTRY("Earth",  150000000000.f, 6378140,5.976E+024, 29800 ) \
PLANETS_ENTRY("Mars",   227940000000.f, 3397200,6.421E+023, 24100 ) \
PLANETS_ENTRY("Jupiter",778330000000.f, 71492000,1.9E+027,  13100 ) \
PLANETS_ENTRY("Saturn", 1429400000000.f,60268000,5.688E+026,    9640 ) \
PLANETS_ENTRY("Uranus", 2870990000000.f,25559000,8.686E+025,    6810 ) \
PLANETS_ENTRY("Neptune",4504300000000.f,24746000,1.024E+026,    5430 ) \
//PLANETS_ENTRY("Pluto",  5913520000000.f,1137000,1.27E+022,  4740 )
//end tuple

/*static*/ double   Orbiter::sDefaultTimeScale          = 60.f * 60.f * 24.f * 10.f; // 10 days
/*static*/ double   Orbiter::sDefaultGravityConstant    = 6.6742e-11;
/*static*/ double   Orbiter::sDrawScale                 = 2e-11;
/*static*/ float    Orbiter::sMaxRadiusMultiplier       = 2.6f;
/*static*/ int      Orbiter::sNumFramesToAvgFft         = 1;
/*static*/ bool     Orbiter::sUseSmoothLines            = true;
/*static*/ bool     Orbiter::sUseTriStripLine           = false;
/*static*/ int      Orbiter::sMinTrailLength            = 47;// in segments
/*static*/ float    Orbiter::sTrailWidth                = 1.4f;
/*static*/ bool     Orbiter::sDrawRealSun               = false;
/*static*/ float    Orbiter::sPlanetGrayScale           = 0.6f;

//
// Orbiter
//
Orbiter::Orbiter()
: Scene()
, mSun(NULL)
, mFollowTargetIndex(0)
, mFollowTarget(NULL)
, mIsFollowCameraEnabled(true)
{
    mEnableFrustumCulling = true; // Scene
    
    for( int i=0; i < TB_COUNT; ++i )
    {
        mTextBox[i] = new TextEntity();
        assert(mTextBox[i] != NULL && "out of memory, how ridiculous");
    }
} 

Orbiter::~Orbiter()
{
    // midi map will get destroyed
}

void Orbiter::setup()
{
    mApp->setUseMayaCam(!mIsFollowCameraEnabled);
    
    mTimeScale = sDefaultTimeScale;
    mDrawScale = Orbiter::sDrawScale;
    mGravityConstant = sDefaultGravityConstant;
    
    //mLightDirection = Vec3f( 0.025f, 0.25f, 1.0f );
	//mLightDirection.normalize();
    
    mMidiMap.init(&mApp->getMidiInput());
    setupMidiMapping();
    
    setupHud();
    
    reset();
}

void Orbiter::resize()
{
    for(BodyList::iterator bodyIt = mBodies.begin(); 
        bodyIt != mBodies.end();
        ++bodyIt)
    {
        (*bodyIt)->resetTrail();
    }
    
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->resize();
    }
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
    params.addParam("Gravity Constant", &mGravityConstant, "step=0.00000000001 keyIncr== keyDecr=-");
    params.addParam("Follow Target", &mFollowTargetIndex, "keyIncr=] keyDecr=[");
    params.addParam("Time Scale", &mTimeScale, "step=86400.0 KeyIncr=. keyDecr=,");
    params.addParam("Max Radius Mult", &Orbiter::sMaxRadiusMultiplier, "step=0.1");
    params.addParam("Frames to Avg", &Orbiter::sNumFramesToAvgFft, "step=1");
    params.addParam("Trails - Smooth", &Orbiter::sUseSmoothLines, "key=s");
    params.addParam("Trails - Ribbon", &Orbiter::sUseTriStripLine, "key=t");
    params.addParam("Trails - LengthFact", &Orbiter::sMinTrailLength, "keyIncr=l keyDecr=;");
    params.addParam("Trails - Width", &Orbiter::sTrailWidth, "keyIncr=w keyDecr=q step=0.1");
    params.addParam("Planet Grayscale", &Orbiter::sPlanetGrayScale, "keyIncr=x keyDecr=z step=0.05");
    //params.addParam("Real Sun Radius", &Orbiter::sDrawRealSun, "key=r");
    //params.addSeparator();
    //params.addParam("Frustum Culling", &mEnableFrustumCulling, "keyIncr=f");
}

void Orbiter::reset()
{
    mElapsedTime = 0.0f;
    
    const float radialEnhancement = 500000.0f;// at true scale the plaents would be invisible
    mBodies.clear(); // this will delete mSun
    
    // sun
    double mass = 1.989E+030;
    float radius = 3800000.0f * mDrawScale * radialEnhancement;
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
    
    double angle;
    Vec3d pos;
    pos.y = 0.0f;
    
    
#define PLANETS_ENTRY(name,orad,brad,mss,ovel) \
    mass = mss;\
    orbitalRadius = orad;\
    orbitalVel = ovel;\
    angle = toRadians(Rand::randFloat(3.0f, 8.0f));\
    pos.x = orbitalRadius * sin ( angle );\
    pos.z = orbitalRadius * cos ( angle );\
    radius = brad * mDrawScale * radialEnhancement;\
    body = new Body(name,\
                    pos,\
                    Vec3d(0.0f, orbitalVel, 0.0f),\
                    radius, \
                    mass, \
                    ColorA(Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale)); \
    body->setup(); \
    mBodies.push_back( body );
    PLANETS_TUPLE
#undef PLANETS_ENTRY
    
    // random comets
    int num_planets = mBodies.size();
    int num_comets = 8;
    //Vec3f orbitalPlaneN = Vec3d( Rand::randFloat(
    double farthestRadius = 0.0f;
    Body* farthestComet = NULL;
    const float minRadius = 1000000.0f * Orbiter::sDrawScale * radialEnhancement;
    
    char name[128];
    
    for( int i = 0; i < num_comets; ++i )
    {
        mass = 1e8 * Rand::randFloat(1.0f, 10.0f);
        radius = Rand::randFloat(minRadius,minRadius*2.0f);
        double angle = Rand::randFloat(2*M_PI);
        orbitalRadius = (Rand::randInt(100000) + 100000 ) * 4e6;
        Vec3d pos( orbitalRadius * sin ( angle ), 0.0f, orbitalRadius * cos ( angle ) );
        //Vec3d orbitalPlaneNormal = pos.normalized();
        snprintf(name,128,"%c%d/%04d/%02d", randInt('A','Z'), num_planets+i, randInt(1000,9999), randInt(300));
        orbitalVel = ( ( rand() % 200 ) + 100 ) * 50.0;
        Body* body = new Body(name, 
                              pos, 
                              /*orbitalPlaneNormal * orbitalVel,*/
                               Vec3d(0.0f, orbitalVel, 0.0f),
                               radius, 
                               mass, 
                               ColorA(0.5f, 0.55f, 0.525f));
        mBodies.push_back(body);
        
        if( orbitalRadius < farthestRadius || farthestRadius == 0.0f )
        {
            farthestComet = body;
            farthestRadius = orbitalRadius;
        }
    }
    
    mFollowTarget = farthestComet;
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
    audioInput.setFftBandCount(512/*mBodies.size()*/);
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
                if( body != mSun )
                {
                    body->applyForceFromBody(*mSun, dt, mGravityConstant);
                }
            }
            
            body->update(dt);
            
            ++i;
        }
    }
    
    updateAudioResponse();
    updateHud();
    
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
    
    const int numBandsPerBody = 2;
    int bodyIndex = 0;
    
    if( fftBuffer )
    {
        for( int i = 0; i < ( bandCount - numBandsPerBody ); i += numBandsPerBody) 
        {
            if( bodyIndex < mBodies.size() )
            {
                float avgFft = 0.0f;
                for( int j = 0; j < numBandsPerBody; ++j )
                {
                    avgFft += fftBuffer[i] / bandCount;
                }
                avgFft /= (float)(numBandsPerBody);
                mBodies[bodyIndex]->applyFftBandValue( avgFft );
                ++bodyIndex;
            }
        }
    }
}

void Orbiter::draw()
{
    glPushMatrix();
    //glEnable( GL_MULTISAMPLE_ARB );
    glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    
    
    Matrix44d matrix = Matrix44d::identity();
    matrix.scale(Vec3d( mDrawScale * getWindowWidth() / 2.0f, 
                        mDrawScale * getWindowHeight() / 2.0f,
                        mDrawScale * getWindowHeight() / 2.0f));
    
    int culled = 0;
    for(BodyList::iterator bodyIt = mBodies.begin(); 
        bodyIt != mBodies.end();
        ++bodyIt)
    {
        Body* body = (*bodyIt);
        if (!mEnableFrustumCulling ||
            isSphereInFrustum( (matrix * body->getPosition()), body->getRadius()*mDrawScale ) )
        {
            body->draw(matrix, true);
        }
        else
        {
            body->draw(matrix, false);
            culled++;
        }
        
        //glPushMatrix();
        //Vec3d pos = matrix * bodyIt->getPosition();
        //glTranslated(pos.x, pos.y, pos.z);
        //glPopMatrix();
    }
    
    //mApp->console() << "culled " << culled << std::endl;
    
    
    if( mIsFollowCameraEnabled )
    {
        Body* cameraLookingAt = mSun;
        Body* cameraAttachedTo = mFollowTarget;
        if( mFollowTargetIndex >= 0 && mFollowTargetIndex < mBodies.size() )
        {
            cameraAttachedTo = mBodies[mBodies.size()-mFollowTargetIndex-1];
            mFollowTarget = cameraAttachedTo;
        }
        
        // put the camera outside the planet
        double offset = cameraAttachedTo->getRadius()*1.2f;
        Vec3d cameraPos = matrix * cameraAttachedTo->getPosition();
        Vec3d targetPos = matrix * cameraLookingAt->getPosition();
        Vec3d toTarget = targetPos - cameraPos;
        Vec3d offsetVec = offset * toTarget.normalized();
        //Matrix44f transform = Matrix44f::createRotation( Vec3f::zAxis(), sinf( (float) getElapsedSeconds() * 0.5f ) * 1.08f );
        //transform.rotate.rotate( Vec3f::zAxis(), sinf( (float) getElapsedSeconds() * 0.5f ) * 1.08f );
        Vec3d up = /*Quatf( Vec3f::zAxis(), sinf( (float) getElapsedSeconds() * 0.5f ) * 1.08f ) */ Vec3f::zAxis();
        //up.rotate( Vec3f::zAxis(), sinf( (float) getElapsedSeconds() * 0.5f ) * 1.08f );
        cameraPos = cameraPos + offsetVec;
        //Vec3f camPos = Vec3f( toCore.x, toCore.y, toCore.z );
        mApp->setCamera( cameraPos, targetPos, up );
    }
    
    glPopMatrix();
    glDisable( GL_LIGHTING );
	glDisable( GL_LIGHT0 );
    
    drawHud();
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

//
// HUD
//

void Orbiter::setupHud()
{
    const float margin = 20.0f;
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->setFont("Menlo", 13.0f);
        mTextBox[i]->setTextColor( ColorA(0.9f,0.9f,0.9f,1.0f) );
        
        switch(i)
        {
            case TB_TOP_LEFT:
                mTextBox[i]->setPosition( Vec3f(margin, margin, 0.0f) );
                break;
            case TB_TOP_RIGHT:
                mTextBox[i]->setPosition( Vec3f(0.0f, margin, 0.0f) );
                mTextBox[i]->setRightJustify( true, margin );
                break;
            case TB_BOT_LEFT:
                mTextBox[i]->setPosition( Vec3f(margin, 0.0f, 0.0f) );
                mTextBox[i]->setBottomJustify( true, margin );
                break;
            case TB_BOT_RIGHT:
                mTextBox[i]->setRightJustify( true, margin );
                mTextBox[i]->setBottomJustify( true, margin );
                break;
            default:
                break;
                
        }
    }
}

void Orbiter::updateHud()
{
    ostringstream oss1;
    
    format timeFormat("SIMULATION TIME: %02d:%02d:%02d:%04.1f");
    int days = mElapsedTime / 86400; 
    int hours = ci::math<double>::fmod(mElapsedTime,86400.0f) / 3600;
    int mins = ci::math<double>::fmod(mElapsedTime,3600.0f) / 60;
    double secs = ci::math<double>::fmod(mElapsedTime,60.0f) + (mElapsedTime - (int)(mElapsedTime));
    timeFormat % days % hours % mins % secs;
    
    format params("TIME SCALE: %-6g : 1\nG CONSTANT: %-8g\nPROJECTION SCALE: 1 : %-8g\n");
    params % mTimeScale % mGravityConstant % mDrawScale;
    
    oss1 << timeFormat << endl << params;
    
    //snprintf(buf, 256, "SIMULATION TIME: %02d:%02d:%02d:%04.1f", days,hours,mins,secs);
    mTextBox[TB_TOP_LEFT]->setText(oss1.str());
    
    ostringstream oss2;
    
    format followCamInfo("DESIGNATION: %s\nORBITAL RADIUS: %.4e km\nORBITAL SPEED: %6.1f m/s\nMASS: %.6e kg");
    followCamInfo % (mFollowTarget->getName()) % (mFollowTarget->getPosition().length() / 1000.0f) % (mFollowTarget->getVelocity().length()) % (mFollowTarget->getMass());
    
    oss2 << followCamInfo;
    
    mTextBox[TB_BOT_RIGHT]->setText(oss2.str());
}

void Orbiter::drawHud()
{
    gl::pushMatrices();
    
    CameraOrtho textCam(0.0f, mApp->getWindowWidth(), mApp->getWindowHeight(), 0.0f, 0.0f, 50.f);
    gl::setMatrices(textCam);
    
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->draw();
    }
    
    gl::popMatrices();
}