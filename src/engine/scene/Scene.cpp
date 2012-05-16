/*
 *  Scene.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include <string>
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"
#include "SimpleGUI.h"
#include "Utils.h"

using namespace ci;
using namespace mowa::sgui;

/*static*/const char* const Scene::kIniLocation = "/Volumes/cruxpod/oculondata/params/";
/*static*/const char* const Scene::kIniExt = ".ini";
/*static*/const int         Scene::kIniDigits = 2;

Scene::Scene(const std::string& name)
: mName(name)
, mApp(NULL)
, mIsRunning(false)
, mIsVisible(false)
, mIsDebug(false)
, mEnableFrustumCulling(false)
, mInterface(NULL)
, mIsFrustumPlaneCached(false)
, mDoReset(false)
{
    // frustum culling
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		mCachedFrustumPlane[i].mNormal = Vec3f::zero();
		mCachedFrustumPlane[i].mPoint = Vec3f::zero();
		mCachedFrustumPlane[i].mDistance = 0.0f;
	}
    
    mDebugParams = params::InterfaceGl( name, Vec2i( 350, 400 ) );
    mDebugParams.show(mIsDebug);
}

Scene::~Scene()
{
    delete mInterface;
}

void Scene::init(OculonApp* app)
{
    mApp = app;
    mInterface = new Interface( mApp, &mApp->getOscServer() );
    mInterface->gui()->addColumn();
    // bind to OculonApp::showInterface(0)
    mInterface->gui()->addButton(mName)->registerCallback( boost::bind(&OculonApp::showInterface, mApp, 0) );
    // bind to Scene::loadInterfaceParams(0)
    mInterface->addParam(CreateBoolParam("active", &mIsVisible)
                         .oscReceiver(mName,"toggle")
                         .oscSender("/oculon/" + mName + "/visible").sendFeedback())->registerCallback( this, &Scene::setRunningByVisibleState );
    mInterface->addButton(CreateTriggerParam("reset", NULL)
                          .oscReceiver(mName))->registerCallback( this, &Scene::onReset );
    mInterface->addParam(CreateBoolParam("visible", &mIsVisible)
                         .oscReceiver(mName).sendFeedback())->registerCallback( this, &Scene::onVisibleChanged );
    mInterface->addParam(CreateBoolParam("running", &mIsRunning)
                         .oscReceiver(mName).sendFeedback())->registerCallback( this, &Scene::onRunningChanged );
    mInterface->addParam(CreateBoolParam("debug", &mIsDebug))->registerCallback( this, &Scene::onDebugChanged );
    mInterface->gui()->addButton("LOAD")->registerCallback( boost::bind(&Scene::loadInterfaceParams, this, 0) );
    mInterface->gui()->addButton("SAVE")->registerCallback( this, &Scene::saveInterfaceParams );
    mInterface->gui()->addSeparator();
    mInterface->gui()->setEnabled(false); // always start hidden
    
    setup();
    setupInterface();
    setupDebugInterface();
}

void Scene::setupDebugInterface()
{
    // add all interface params as debug params
    for (Interface::const_iterator it = mInterface->paramsBegin();
         it != mInterface->paramsEnd();
         ++it)
    {
        OscParam* param = (*it);
        Control* control = param->getControl();
        
        switch( control->type )
        {
            case Control::FLOAT_VAR:
                mDebugParams.addParam( control->name, static_cast<FloatVarControl*>(control)->var );
                break;
            case Control::DOUBLE_VAR:
                mDebugParams.addParam( control->name, static_cast<DoubleVarControl*>(control)->var );
                break;
            case Control::INT_VAR:
                mDebugParams.addParam( control->name, static_cast<IntVarControl*>(control)->var );
                break;
            case Control::BOOL_VAR:
                mDebugParams.addParam( control->name, static_cast<BoolVarControl*>(control)->var );
                break;
            case Control::SEPARATOR:
                mDebugParams.addSeparator();
                break;
            default:
                break;
        }
    }
}

void Scene::update(double dt)
{
    if( mDoReset )
    {
        mDoReset = false;
        reset();
    }
    if( mEnableFrustumCulling )
    {
        mIsFrustumPlaneCached = false;
        calcNearAndFarClipCoordinates( getCamera() );
    }
    
    assert(mInterface);
    mInterface->update();
}

void Scene::drawInterface()
{
    mInterface->draw();
}

void Scene::drawDebug()
{
    mDebugParams.draw();
}

const Camera& Scene::getCamera() const
{
    return mApp->getMayaCam();
}

bool Scene::toggleActiveVisible()
{
    setVisible(!mIsVisible);
    setRunning(mIsVisible);
    
    return false;
}

bool Scene::setRunningByVisibleState()
{
    setVisible(mIsVisible);
    setRunning(mIsVisible);
    
    return false;
}

void Scene::setVisible(bool visible)
{
    mIsVisible = visible;
    onVisibleChanged();
}

bool Scene::onVisibleChanged()
{
    mDebugParams.show(mIsDebug && mIsVisible);
    handleVisibleChanged();
    return false;
}

void Scene::setRunning(bool running)
{ 
    mIsRunning = running;
    onRunningChanged();
}

bool Scene::onRunningChanged()
{
    handleRunningChanged();
    return false;
}

void Scene::setDebug(bool debug)
{
    mIsDebug = debug;
    onDebugChanged();
}

bool Scene::onDebugChanged()
{
    mDebugParams.show(mIsDebug && mIsVisible);
    handleDebugChanged();
    return false;
}

void Scene::showInterface(bool show)
{
    assert(mInterface);
    mInterface->gui()->setEnabled(show);
    if( show )
    {
        mInterface->createControlInterface(mName);
    }
}

bool Scene::saveInterfaceParams() 
{
    const string pathBase = kIniLocation + getName() + kIniExt;
    fs::path filePath = Utils::getUniquePath(pathBase, kIniDigits, "");
    mInterface->gui()->save(filePath.string());
    return false;//callback
}

bool Scene::loadInterfaceParams(const int index)
{
    std::stringstream filePath;
    filePath << kIniLocation << getName() << setw(kIniDigits) << setfill('0') << index << kIniExt;
    mInterface->gui()->load(filePath.str());
    return false;//callback
}

#pragma MARK: Frustum Culling
void Scene::calcNearAndFarClipCoordinates( const Camera &cam )
{
	Vec3f ntl, ntr, nbl, nbr;
	cam.getNearClipCoordinates( &ntl, &ntr, &nbl, &nbr );
    
	Vec3f ftl, ftr, fbl, fbr;
	cam.getFarClipCoordinates( &ftl, &ftr, &fbl, &fbr );
	
	if( ! mIsFrustumPlaneCached )
    {
		calcFrustumPlane( mCachedFrustumPlane[TOP].mNormal, mCachedFrustumPlane[TOP].mPoint, mCachedFrustumPlane[TOP].mDistance, ntr, ntl, ftl );
		calcFrustumPlane( mCachedFrustumPlane[BOT].mNormal, mCachedFrustumPlane[BOT].mPoint, mCachedFrustumPlane[BOT].mDistance, nbl, nbr, fbr );
		calcFrustumPlane( mCachedFrustumPlane[LEF].mNormal, mCachedFrustumPlane[LEF].mPoint, mCachedFrustumPlane[LEF].mDistance, ntl, nbl, fbl );
		calcFrustumPlane( mCachedFrustumPlane[RIG].mNormal, mCachedFrustumPlane[RIG].mPoint, mCachedFrustumPlane[RIG].mDistance, ftr, fbr, nbr );
		calcFrustumPlane( mCachedFrustumPlane[NEA].mNormal, mCachedFrustumPlane[NEA].mPoint, mCachedFrustumPlane[NEA].mDistance, ntl, ntr, nbr );
		calcFrustumPlane( mCachedFrustumPlane[FARP].mNormal, mCachedFrustumPlane[FARP].mPoint, mCachedFrustumPlane[FARP].mDistance, ftr, ftl, fbl );
	}
	
	mIsFrustumPlaneCached = true;
}

// need 'const'?
void Scene::calcFrustumPlane( Vec3f &fNormal, Vec3f &fPoint, float &fDist, const Vec3f &v1, const Vec3f &v2, const Vec3f &v3 )
{
	Vec3f aux1, aux2;
    
	aux1 = v1 - v2;
	aux2 = v3 - v2;
    
	fNormal = aux2.cross( aux1 );
	fNormal.normalize();
	fPoint.set( v2 );
	fDist = -( fNormal.dot( fPoint ) );
}

bool Scene::isPointInFrustum( const Vec3f &loc )
{
	float d;
	bool result = true;
    
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		d = mCachedFrustumPlane[i].mDistance + mCachedFrustumPlane[i].mNormal.dot( loc );
		if( d < 0 )
			return( false );
	}
    
	return( result );
}


bool Scene::isSphereInFrustum( const Vec3f &loc, float radius )
{
	float d;
	bool result = true;
	
	for(int i=0; i<SIDE_COUNT; i++ )
    {
		d = mCachedFrustumPlane[i].mDistance + mCachedFrustumPlane[i].mNormal.dot( loc );
		if( d < -radius )
			return( false );
	}
	
	return( result );
}


bool Scene::isBoxInFrustum( const Vec3f &loc, const Vec3f &size )
{
	float d;
	int out, in;
	bool result = true;
	
	Vec3f vertex[8];
	
	vertex[0] = Vec3f( loc.x + size.x, loc.y + size.y, loc.z + size.z );
	vertex[1] = Vec3f( loc.x + size.x, loc.y - size.y, loc.z + size.z );
	vertex[2] = Vec3f( loc.x + size.x, loc.y - size.y, loc.z - size.z );
	vertex[3] = Vec3f( loc.x + size.x, loc.y + size.y, loc.z - size.z );
	vertex[4] = Vec3f( loc.x - size.x, loc.y + size.y, loc.z + size.z );
	vertex[5] = Vec3f( loc.x - size.x, loc.y - size.y, loc.z + size.z );
	vertex[6] = Vec3f( loc.x - size.x, loc.y - size.y, loc.z - size.z );
	vertex[7] = Vec3f( loc.x - size.x, loc.y + size.y, loc.z - size.z );
	
	for( int i=0; i<SIDE_COUNT; i++ )
    {
		out = 0;
		in = 0;
        
		for( int k=0; k<8 && ( in==0 || out==0 ); k++ )
        {
			d = mCachedFrustumPlane[i].mDistance + mCachedFrustumPlane[i].mNormal.dot( vertex[k] );
			
			if( d < 0 )
				out ++;
			else
				in ++;
		}
        
		if( !in )
			return( false );
	}
	
	return( result );
}
