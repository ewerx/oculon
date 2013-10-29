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
#include "Resources.h"
#include "Interface.h"
#include "Star.h"
#include "Planet.h"

using namespace ci;
using namespace ci::app;
using namespace boost;
using namespace std;

//TODO: read from data file (orbRad,    bodyRad, mass,          oVel,   rot,        tex)
#define PLANETS_TUPLE \
PLANETS_ENTRY("Mercury",57900000000.f,  2440000,3.33E+023,      47900,  1.240E-06,   RES_ORBITER_MERCURY ) \
PLANETS_ENTRY("Venus",  108000000000.f, 6050000,4.869E+024,     35000,  -2.99E-07,  RES_ORBITER_VENUS ) \
PLANETS_ENTRY("Earth",  150000000000.f, 6378140,5.976E+024,     29800,  7.30E-05,   RES_ORBITER_EARTH ) \
PLANETS_ENTRY("Mars",   227940000000.f, 3397200,6.421E+023,     24100,  7.09E-05,   RES_ORBITER_MARS ) \
PLANETS_ENTRY("Jupiter",778330000000.f, 71492000,1.898E+027,    13070,  1.76E-04,   RES_ORBITER_JUPITER ) \
PLANETS_ENTRY("Saturn", 1429400000000.f,60268000,5.688E+026,    9640,   1.63E-04,   RES_ORBITER_SATURN ) \
PLANETS_ENTRY("Uranus", 2870990000000.f,25559000,8.686E+025,    6810,   -1.01E-04,   RES_ORBITER_URANUS ) \
PLANETS_ENTRY("Neptune",4504300000000.f,24746000,1.024E+026,    5430,   1.08E-04,   RES_ORBITER_NEPTUNE ) \
//PLANETS_ENTRY("Pluto",  5913520000000.f,1137000,1.27E+022,  4740 )
//end tuple

/*static*/ double   Orbiter::sDefaultTimeScale          = 60.f * 60.f * 24.f * 10.f; // 10 days
/*static*/ double   Orbiter::sDefaultGravityConstant    = 6.6742e-11;
/*static*/ double   Orbiter::sDrawScale                 = 2e-12;
/*static*/ bool     Orbiter::sUseSmoothLines            = true;
/*static*/ bool     Orbiter::sUseTriStripLine           = false;
/*static*/ int      Orbiter::sMinTrailLength            = 47;// in segments
/*static*/ float    Orbiter::sTrailWidth                = 1.4f;
/*static*/ bool     Orbiter::sDrawRealSun               = false;
/*static*/ float    Orbiter::sPlanetGrayScale           = 1.0f;

//
// Orbiter
//
Orbiter::Orbiter()
: Scene("orbiter")
, mSun(NULL)
, mFollowTargetIndex(0)
, mFollowTarget(NULL)
, mCamType(CAM_FOLLOW)
, mExoStar(NULL)
{
    mEnableFrustumCulling = true; // Scene
    
    for( int i=0; i < TB_COUNT; ++i )
    {
        mTextBox[i] = new TextEntity(this);
        assert(mTextBox[i] != NULL && "out of memory, how ridiculous");
    }
    
    // load textures
    int i = 0;
#define PLANETS_ENTRY(name,orad,brad,mss,ovel,rot,tex) \
mTextures[i++] = loadImage( loadResource(tex) );
    PLANETS_TUPLE
#undef PLANETS_ENTRY
} 

Orbiter::~Orbiter()
{
    removeBodies();
    
    // midi map will get destroyed
    for( int i=0; i < TB_COUNT; ++i )
    {
        delete mTextBox[i];
        mTextBox[i] = NULL;
    }
}

void Orbiter::setup()
{
    Scene::setup();
    
    mDrawHud = false;
    mDrawLabels = true;
    mDrawTrails = true;
    
    mFrequencySpread = 5;
    mMinRadiusMultiplier = 1.5f;
    mMaxRadiusMultiplier = 5.0f;
    mFalloff = 0.18f;
    
    mCam.setEyePoint( Vec3f(0.0f, 0.0f, 750.0f) );
	mCam.setCenterOfInterestPoint( Vec3f::zero() );
	mCam.setPerspective( 65.0f, mApp->getViewportAspectRatio(), 1.0f, 200000.0f );
    
    mCamType = CAM_CATALOG;
    
    mTimeScale = sDefaultTimeScale;
    mDrawScale = Orbiter::sDrawScale;
    mGravityConstant = sDefaultGravityConstant;
    
    mFontHud            = Font( "Menlo", 13 );
    mFontLabel          = Font( "Menlo", 10 );
    mTextureFontHud     = gl::TextureFont::create( mFontHud );
	mTextureFontLabel	= gl::TextureFont::create( mFontLabel );
    
    mLabelBrightnessByAudio = 0.0f;
    
    setupHud();
    
    reset();
}

void Orbiter::resize()
{
    Scene::resize();
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

void Orbiter::setupInterface()
{
    mInterface->addEnum(CreateEnumParam("Camera", (int*)(&mCamType) )
                        .maxValue(CAM_COUNT)
                        .oscReceiver(getName(), "camera")
                        .isVertical());
    
    mInterface->addButton(CreateTriggerParam("Next Target", NULL)
                          .oscReceiver(mName,"nexttarget"))->registerCallback( this, &Orbiter::nextTarget );
    mInterface->addButton(CreateTriggerParam("Prev Target", NULL)
                          .oscReceiver(mName,"prevtarget"))->registerCallback( this, &Orbiter::prevTarget );
    
    mInterface->addParam(CreateBoolParam("Show Hud", &mDrawHud)
                         .oscReceiver(getName(), "showhud"));
    mInterface->addParam(CreateBoolParam("Show Labels", &mDrawLabels)
                         .oscReceiver(getName(), "showlabels"));
    mInterface->addParam(CreateBoolParam("Show Trails", &mDrawTrails)
                         .oscReceiver(getName(), "showtrails"));
    
    mInterface->addParam(CreateIntParam("Freq Spread", &mFrequencySpread)
                         .minValue(1)
                         .maxValue(512)
                         .oscReceiver(mName,"freqspread"));
    mInterface->addParam(CreateFloatParam("Min Rad Mult", &mMinRadiusMultiplier)
                         .minValue(1.0f)
                         .maxValue(3.0f)
                         .oscReceiver(mName,"minradmult"));
    mInterface->addParam(CreateFloatParam("Max Rad Mult", &mMaxRadiusMultiplier)
                         .minValue(1.0f)
                         .maxValue(20.0f)
                         .oscReceiver(mName,"maxradmult"));
    mInterface->addParam(CreateFloatParam("Falloff", &mFalloff)
                         .maxValue(1.0f)
                         .oscReceiver(mName,"falloff"));
    
    mInterface->addParam(CreateFloatParam("Label FFT", &mLabelBrightnessByAudio)
                         .oscReceiver(mName,"labelfft"));
}


void Orbiter::setupDebugInterface()
{
    Scene::setupDebugInterface();
    
    mDebugParams.addParam("Gravity Constant", &mGravityConstant, "step=0.00000000001 keyIncr== keyDecr=-");
    //mDebugParams.addParam("Follow Target", &mFollowTargetIndex, "keyIncr=] keyDecr=[");
    mDebugParams.addParam("Time Scale", &mTimeScale, "step=86400.0 KeyIncr=. keyDecr=,");
    mDebugParams.addParam("Trails - Smooth", &Orbiter::sUseSmoothLines, "key=t");
    mDebugParams.addParam("Trails - Ribbon", &Orbiter::sUseTriStripLine);
    mDebugParams.addParam("Trails - LengthFact", &Orbiter::sMinTrailLength, "");
    mDebugParams.addParam("Trails - Width", &Orbiter::sTrailWidth, "step=0.1");
    mDebugParams.addParam("Planet Grayscale", &Orbiter::sPlanetGrayScale, "step=0.05");
    //mDebugParams.addParam("Real Sun Radius", &Orbiter::sDrawRealSun, "key=r");
    //mDebugParams.addSeparator();
    //mDebugParams.addParam("Frustum Culling", &mEnableFrustumCulling, "keyIncr=f");
}

void Orbiter::reset()
{
    createSolSystem();
    
    mElapsedTime = 0.0f;
}

void Orbiter::createSolSystem()
{
    removeBodies();
    
    const float radialEnhancement = 500000.0f;// at true scale the planets would be invisible
    
    // sun
    double mass = 1.989E+030;
    float radius = (float)(3800000.0f * mDrawScale * radialEnhancement);
    double orbitalRadius = 0.0f;
    double orbitalVel = 0.0f;
    double rotationSpeed = 0.0f;
    mSun = new Sun(this,
                   "Sol",
                   Vec3d::zero(),
                   Vec3d::zero(),
                   radius,
                   rotationSpeed,
                   mass,
                   ColorA(1.0f, 1.0f, 1.0f));
    mSun->setup();
    mSun->setLabelVisible(false);
    mBodies.push_back(mSun);
    
    mExoStar = NULL;
    
    Body* body;
    
    double angle;
    Vec3d pos;
    pos.y = 0.0f;
    
    int i = 0;
    
#define PLANETS_ENTRY(name,orad,brad,mss,ovel,rot,tex) \
    mass = mss;\
    orbitalRadius = orad;\
    orbitalVel = ovel;\
    rotationSpeed = rot;\
    angle = toRadians(Rand::randFloat(3.0f, 8.0f));\
    pos.y = orbitalRadius * sin ( angle );\
    pos.z = orbitalRadius * cos ( angle );\
    radius = brad * mDrawScale * radialEnhancement;\
    body = new Body(this, name,\
                    pos,\
                    Vec3d(orbitalVel, 0.0f, 0.0f),\
                    radius, \
                    rotationSpeed, \
                    mass, \
                    ColorA(Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale), \
                    mTextures[i++]); \
    body->setup(); \
    mBodies.push_back( body );
    PLANETS_TUPLE
#undef PLANETS_ENTRY
    
    // random comets
    int num_planets = mBodies.size();
    int num_comets = 12;
    //Vec3f orbitalPlaneN = Vec3d( Rand::randFloat(
    //double farthestRadius = 0.0f;
    //Body* farthestComet = NULL;
    const float minRadius = (float)(1000000.0f * Orbiter::sDrawScale * radialEnhancement);
    
    char name[128];
    
    for( int i = 0; i < num_comets; ++i )
    {
        mass = 1e8 * Rand::randFloat(1.0f, 10.0f);
        radius = Rand::randFloat(minRadius,minRadius*2.0f);
        double angle = Rand::randFloat(2.0f*(float)M_PI);
        orbitalRadius = (Rand::randInt(100000) + 100000) * 4e6;
        Vec3d pos( 0.0f, orbitalRadius * sin ( angle ), orbitalRadius * cos ( angle ) );
        //Vec3d orbitalPlaneNormal = pos.normalized();
        snprintf(name,128,"%c%d/%04d/%02d", randInt('A','Z'), num_planets+i, randInt(1000,9999), randInt(300));
        orbitalVel = ( ( rand() % 200 ) + 100 ) * 50.0;
        Body* body = new Body(this,
                              name,
                              pos,
                              /*orbitalPlaneNormal * orbitalVel,*/
                              Vec3d(orbitalVel, 0.0f, 0.0f),
                              radius,
                              0.0000000001f,
                              mass,
                              ColorA(0.5f, 0.5f, 0.5f));
        body->setup();
        mBodies.push_back(body);
    }
    
    mFollowTargetIndex = Rand::randInt(9,mBodies.size()-1);
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
}

void Orbiter::createSystem( Star* star )
{
    if( star == NULL )
    {
        createSolSystem();
        return;
    }
    
    int numPlanets = star->mPlanets.size();
    if( numPlanets == 0 )
    {
        console() << "[orbiter] ERROR creating system: star has no planets\n";
        return;
    }
    
    removeBodies();
    
    const float radialEnhancement = 500000.0f;// at true scale the planets would be invisible
    
    // star
    double mass = star->mPlanets[0]->mStellarMass * 1.989E+030; // solar-mass
    float radius = (float)(3800000.0f * mDrawScale * radialEnhancement);
    double rotationSpeed = 0.0f;
    mSun = new Sun(this,
                   star->mName,
                   Vec3d::zero(),
                   Vec3d::zero(),
                   radius,
                   rotationSpeed,
                   mass,
                   ColorA(1.0f, 1.0f, 1.0f));
    mSun->setup();
    mSun->setLabelVisible(false);
    mBodies.push_back(mSun);
    
    mExoStar = star;
    
    Body* body;
    
    double orbitalRadius;
    double orbitalVel;
    double angle;
    Vec3d pos;
    pos.y = 0.0f;
    
    string name;
    
    for( int i=0; i < numPlanets; ++i )
    {
        Planet* planet = star->mPlanets[i];
        mass = planet->mMass * 1.898E+027; // Jupiter-mass
        orbitalRadius = planet->mSemiMajorAxis * 149598000000; // AU -> M
        // this assumes near circular orbit
        // v0 = 2pi * semiMajorAxis / orbPeriod
        orbitalVel = 2*M_PI * orbitalRadius / ( planet->mOrbitalPeriod * 24 * 3600 );
        rotationSpeed = 0.0f;
        angle = toRadians(Rand::randFloat(3.0f, 8.0f)); // inclination?
        pos = mSun->getPosition();
        pos.y += orbitalRadius * sin ( angle );
        pos.z += orbitalRadius * cos ( angle );
        radius = planet->mRadius*71492000 * mDrawScale * radialEnhancement; // Jupiter-radius
        body = new Body(this, name,
                        pos,
                        Vec3d(orbitalVel, 0.0f, 0.0f),
                        radius, 
                        rotationSpeed, 
                        mass, 
                        ColorA(Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale, Orbiter::sPlanetGrayScale),
                        mTextures[i++]); 
        body->setup(); 
        mBodies.push_back( body );
    }
    
    mFollowTargetIndex = Rand::randInt(0,mBodies.size()-1);
    
    mElapsedTime = 0.0f;
}

void Orbiter::removeBodies()
{
    for(BodyList::iterator bodyIt = mBodies.begin();
        bodyIt != mBodies.end();
        ++bodyIt)
    {
        delete (*bodyIt);
    }
    mBodies.clear();
    mSun = NULL;
    mFollowTarget = NULL;
    mFollowTargetIndex = 0;
}

void Orbiter::update(double dt)
{
    dt *= mTimeScale;
    mElapsedTime += dt;

    // debug info
    char buf[256];
    //snprintf(buf, 256, "orbiter cam: %d", mCamType );
    //mApp->getInfoPanel().addLine(buf, Color(0.5f, 0.5f, 0.75f));
    
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
    updateCam();
    
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
            //reset();
            handled = false;
            break;
        case 'c':
            mCamType = (mCamType == CAM_FOLLOW) ? CAM_MANUAL : CAM_FOLLOW;
            mApp->setUseMayaCam( mCamType == CAM_MANUAL );
            break;
        case '[':
            prevTarget();
            break;
        case ']':
            nextTarget();
            break;
        case 'b':
            mCamType = (mCamType == CAM_FOLLOW) ? CAM_BINNED : CAM_FOLLOW;
            mApp->setUseMayaCam( false );
            handled = false;
            break;
        case 'o':
            toggleActiveVisible();
            handled = false;
            break;
        default:
            handled = false;
            break;
    }
    
    return handled;
}

bool Orbiter::prevTarget()
{
    if( --mFollowTargetIndex < 4 )
        mFollowTargetIndex = mBodies.size()-1;
    
    return false;
}

bool Orbiter::nextTarget()
{
    if( ++mFollowTargetIndex == mBodies.size() )
        mFollowTargetIndex = 4;
}

//
//
//
void Orbiter::updateAudioResponse()
{
    AudioInput& audioInput = mApp->getAudioInput();
    
    //int32_t dataSize = audioInput.getFft()->getBinSize();
    //const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();

    const int numBandsPerBody = mFrequencySpread;//dataSize / mBodies.size();
    
    int minBand = 0;
    int maxBand = numBandsPerBody;
    
    for(BodyList::iterator bodyIt = mBodies.begin();
        bodyIt != mBodies.end();
        ++bodyIt)
    {
        float volume = audioInput.getAverageVolumeByFrequencyRange( minBand, maxBand ) * mGain;
        
        (*bodyIt)->applyFftBandValue( volume );
        
        minBand += numBandsPerBody;
        maxBand += numBandsPerBody;
    }
    
    //
//    std::shared_ptr<float> fftDataRef = audioInput.getFftDataRef();
//    
//    unsigned int bandCount = audioInput.getFftBandCount();
//    float* fftBuffer = fftDataRef.get();
//    
//    const int numBandsPerBody = 2;
//    int bodyIndex = 0;
//    
//    if( fftBuffer )
//    {
//        for( int i = 0; i < ( bandCount - numBandsPerBody ); i += numBandsPerBody) 
//        {
//            if( bodyIndex < mBodies.size() )
//            {
//                float avgFft = 0.0f;
//                for( int j = 0; j < numBandsPerBody; ++j )
//                {
//                    avgFft += fftBuffer[i] / bandCount;
//                }
//                avgFft /= (float)(numBandsPerBody);
//                mBodies[bodyIndex]->applyFftBandValue( avgFft );
//                ++bodyIndex;
//            }
//        }
//    }
}

void Orbiter::updateCam()
{
    mScaleMatrix = Matrix44d::identity();
    mScaleMatrix.scale(Vec3d( mDrawScale * mApp->getViewportWidth() / 2.0f,
                       mDrawScale * mApp->getViewportHeight() / 2.0f,
                       mDrawScale * mApp->getViewportHeight() / 2.0f));
    
    if( CAM_BINNED == mCamType )
    {
        mCam.lookAt( Vec3d(0,6000,0), Vec3d(0,0,0), Vec3d(0,0,1) );
    }
    else if( CAM_FOLLOW == mCamType )
    {
        Body* cameraLookingAt = mSun;
        Body* cameraAttachedTo = mFollowTarget;
        if( mFollowTargetIndex >= 0 && mFollowTargetIndex < mBodies.size() )
        {
            cameraAttachedTo = mBodies[mFollowTargetIndex];
            mFollowTarget = cameraAttachedTo;
        }
        
        // put the camera outside the planet
        double offset = cameraAttachedTo->getRadius()*1.2f;
        Vec3d cameraPos = mScaleMatrix * cameraAttachedTo->getPosition();
        Vec3d targetPos = mScaleMatrix * cameraLookingAt->getPosition();
        Vec3d toTarget = targetPos - cameraPos;
        Vec3d offsetVec = offset * toTarget.normalized();
        Vec3d up = Vec3f::zAxis();
        
#if 0//rotating camera
        float rotationSpeed = 5;
        static Vec3f currentAngles = Vec3f::zero();
        static Vec3f targetAngles = Vec3f::zero();
        float lerpSpeed = 1;
        
        targetAngles.x += toRadians(getElapsedSeconds() * rotationSpeed);
        targetAngles.y += toRadians(getElapsedSeconds() * rotationSpeed);
        targetAngles.y = ci::math<float>::clamp(targetAngles.y,-M_PI,M_PI);
        
        currentAngles = lerp<Vec3f>(currentAngles,targetAngles,mApp->getElapsedSecondsThisFrame()*lerpSpeed);
        
        Quatf xQuaternion(Vec3f::zAxis(), currentAngles.x);
        Quatf yQuaternion(Vec3f::xAxis(), currentAngles.y);
        
        up = up * (xQuaternion * yQuaternion);
#endif
        
        cameraPos = cameraPos + offsetVec;
        mCam.lookAt( cameraPos, targetPos, up );
    }
}

void Orbiter::draw()
{
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::enableAlphaBlending();
    gl::pushMatrices();
    
    gl::setMatrices(getCamera());
    
    if( mExoStar )
    {
        gl::translate( mExoStar->mPos );
    }
    
    if( mDrawTrails )
    {
        glEnable( GL_LINE_SMOOTH );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
        glLineWidth(Orbiter::sTrailWidth);
        
        for(BodyList::iterator bodyIt = mBodies.begin();
            bodyIt != mBodies.end();
            ++bodyIt)
        {
            (*bodyIt)->drawTrail();
        }
    }
    
    glEnable( GL_POLYGON_SMOOTH );
    glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	
	GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    
    
    // frustum culling
    BodyList visibleBodies;
    int culled = 0;
    for(BodyList::iterator bodyIt = mBodies.begin();
        bodyIt != mBodies.end();
        ++bodyIt)
    {
        Body* body = (*bodyIt);
        if (!mEnableFrustumCulling ||
            isSphereInFrustum( (mScaleMatrix * body->getPosition()), body->getBaseRadius()/3.0f ) )
        {
            body->draw(mScaleMatrix, true);
        }
        else
        {
            body->draw(mScaleMatrix, false);
            culled++;
        }
    }
    
	glDisable( GL_LIGHT0 );
    glDisable( GL_LIGHTING );
    
    gl::popMatrices();
    
    if( mDrawHud )
    {
        drawHud();
    }
}

#pragma mark HUD
//
// HUD
//

void Orbiter::setupHud()
{
    const float margin = 25.0f;
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->setFont("Menlo", 13.0f);
        mTextBox[i]->setTextColor( ColorA(1.0f,1.0f,1.0f,1.0f) );
        
        switch(i)
        {
            case TB_TOP_LEFT:
                mTextBox[i]->setPosition( Vec3f(margin, margin, 0.0f) );
                break;
            case TB_TOP_RIGHT:
                mTextBox[i]->setPosition( Vec3f(0.0f, margin+40.f, 0.0f) );
                mTextBox[i]->setRightJustify( true, margin );
                mTextBox[i]->setFont("Menlo", 10.0f);
                //mTextBox[i]->setTextColor( ColorA(1.0f,1.0f,1.0f,1.0f));
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
    // TOP LEFT
    ostringstream oss1;
    
    format timeFormat("SIMULATION TIME: %02dd%02dh%02dm%04.1fs");
    int days = (int)(mElapsedTime / 86400.0f); 
    int hours = (int)(ci::math<double>::fmod(mElapsedTime,86400.0f) / 3600.f);
    int mins = (int)(ci::math<double>::fmod(mElapsedTime,3600.0f) / 60);
    double secs = ci::math<double>::fmod(mElapsedTime,60.0f) + (mElapsedTime - (int)(mElapsedTime));
    timeFormat % days % hours % mins % secs;
    
    format params("TIME SCALE: %-6g : 1\nG CONSTANT: %-8g\nPROJECTION SCALE: 1 / %-8g\n");
    params % mTimeScale % mGravityConstant % mDrawScale;
    
    oss1 << timeFormat << endl << params;
    
    //snprintf(buf, 256, "SIMULATION TIME: %02d:%02d:%02d:%04.1f", days,hours,mins,secs);
    mTextBox[TB_TOP_LEFT]->setText(oss1.str());
    
    // BOTTOM RIGHT
    ostringstream oss2;
    
    if( mFollowTarget )
    {
        format followCamInfo("%s\nOr: %.4e km\nOv: %6.1f m/s\nM: %.6e kg");
        followCamInfo % (mFollowTarget->getName()) % (mFollowTarget->getPosition().length() / 1000.0f) % (mFollowTarget->getVelocity().length()) % (mFollowTarget->getMass());
    
        oss2 << followCamInfo;
    
        mTextBox[TB_BOT_RIGHT]->setText(oss2.str());
    }
    
    // TOP RIGHT
    ostringstream oss3;
    format planetInfo("%.8e\n%.8e\n");
    
        
//    for(BodyList::iterator bodyIt = mBodies.begin(); 
//        bodyIt != mBodies.end();
//        ++bodyIt)
//    {
//        Body* body = (*bodyIt);
    for( int i=0; i < NUM_PLANETS; ++i )
    {
        Body* body = mBodies[i];
        if( body )
        {
            oss3 << format(planetInfo) % body->getAcceleration() % body->getPosition().length();
        }
    }
    
    mTextBox[TB_TOP_RIGHT]->setText(oss3.str());
}

void Orbiter::drawHud()
{
    gl::pushMatrices();
    
    gl::disableDepthRead();
    glDisable(GL_LIGHTING);
    gl::enableAlphaBlending();
    
    CameraOrtho textCam(0.0f, mApp->getViewportWidth(), mApp->getViewportHeight(), 0.0f, 0.0f, 50.f);
    gl::setMatrices(textCam);
    
    gl::color( 1.0f, 1.0f, 1.0f, 0.9f );
    
    for( int i = 0; i < TB_COUNT; ++i )
    {
        //mTextBox[i]->draw();
        if( i == TB_TOP_RIGHT )
        {
            mTextureFontLabel->drawString(mTextBox[i]->getText(), Vec2f(mTextBox[i]->getPosition().x,mTextBox[i]->getPosition().y));
        }
        else
        {
            mTextureFontHud->drawString(mTextBox[i]->getText(), Vec2f(mTextBox[i]->getPosition().x,mTextBox[i]->getPosition().y));
        }
    }
    
    const float width = 200.0f;
    const float height = 100.0f;
    const float space = 10.0f;
    const float left = 20.0f;
    const float top = mApp->getViewportHeight() - 20.0f - (height*2.0f) - space;
    drawHudSpectrumAnalyzer(left,top+height+space,width,height);
    
    gl::popMatrices();
}


void Orbiter::drawHudSpectrumAnalyzer(float left, float top, float width, float height)
{
    AudioInput& audioInput = mApp->getAudioInput();
    //int32_t bandCount = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    uint16_t bandsToShow = math<int>::max( 200, (width / 2.0f) );
    
    const float gap = 1.0f;
    const float barWidth = width / (bandsToShow * (1.0f + gap));
    const float bumpUp = 1.0f;
    const float space = gap + barWidth;
    
    gl::pushMatrices();
    glTranslatef(left,top,0.0f);
    
    for( int i = 0; i < bandsToShow; i++ ) 
    {
        if( (i * space + barWidth) > width )
            break;
        
        float barY = fftLogData[i].y * height * mGain;
        barY = math<float>::min( barY, height );
        glBegin( GL_QUADS );
        glColor3f( 0.8f,0.8f,0.8f );
        glVertex2f( i * space, height-3.f );
        glVertex2f( i * space + barWidth, height-bumpUp );
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex2f( i * space + barWidth, height - barY-bumpUp );
        glVertex2f( i * space, height - barY-3.f );
        glEnd();
    }
    
    float shade = 0.6f;
    gl::color( ColorA(shade,shade,shade,0.75f) );
    glLineWidth(2.0f);
    gl::drawStrokedRect(Rectf(0.0f, 0.0f, width, height));
    
    gl::popMatrices();
}

const Camera& Orbiter::getCamera()
{
    switch( mCamType )
    {
        case CAM_FOLLOW:
        case CAM_BINNED:
            return mCam;
            
        case CAM_CATALOG:
        {
            Scene* scene = mApp->getScene("catalog");
            
            if( scene && scene->isRunning() )
            {
                return scene->getCamera();
            }
            else
            {
                return mCam;
            }
        }
            
        case CAM_MANUAL:
            return mApp->getMayaCam();
            
        default:
            return mApp->getMayaCam();
    }
}