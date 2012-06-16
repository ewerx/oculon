/*
 *  Scene.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-17.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#ifndef __SCENE_H__
#define __SCENE_H__

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"

// fwd decl
class OculonApp;
class Interface;


class Scene
{
public:
    Scene(const std::string& name);
    virtual ~Scene();
    
    void init(OculonApp* app);
    
    virtual void reset() {}
    virtual void shutdown() {}
    virtual void resize() {}
    virtual void update(double dt);
    virtual void draw() {}
    virtual void drawInterface();
    virtual void drawDebug();
    
    virtual bool handleKeyDown( const ci::app::KeyEvent& keyEvent )         { return false; }
    virtual void handleMouseDown( const ci::app::MouseEvent& mouseEvent )   { }
	virtual void handleMouseUp( const ci::app::MouseEvent& event)           { }
	virtual void handleMouseDrag( const ci::app::MouseEvent& event )        { }
    virtual void handleMouseMove( const ci::app::MouseEvent& event )        { }
    virtual void handleRunningChanged()                                     { }
    virtual void handleVisibleChanged()                                     { }
    virtual void handleDebugChanged()                                       { }
    
    virtual const ci::Camera& getCamera() const;
    
    OculonApp* getApp() const       { return mApp; }
    const std::string& getName()    { return mName; }
    
    Interface* getInterface()                       { return mInterface; }
    ci::params::InterfaceGl* getDebugInterface()    { return &mDebugParams; }
    
    bool isRunning() const          { return mIsRunning; }
    bool isVisible() const          { return mIsVisible; }
    bool isDebug() const            { return mIsDebug; }
    
    void setRunning(bool running);
    void setVisible(bool visible);
    void setDebug(bool debug);
    void showInterface(bool show);
    bool toggleActiveVisible();
    bool setRunningByVisibleState();
    
    // callbacks
    bool onDebugChanged();
    bool onVisibleChanged();
    bool onRunningChanged();
    bool onReset()  { mDoReset = true; return false; }
    
    // frustum culling
    bool isFrustumCullingEnabled()  { return mEnableFrustumCulling; }
    void setFrustumCulling( bool enabled ) { mEnableFrustumCulling = enabled; }
    bool isPointInFrustum( const ci::Vec3f &loc );
	bool isSphereInFrustum( const ci::Vec3f &loc, float radius );
	bool isBoxInFrustum( const ci::Vec3f &loc, const ci::Vec3f &size );
    
protected:
    
    virtual void setup() {}
    virtual void setupInterface() {}
    virtual void setupDebugInterface();
    
    bool saveInterfaceParams();
    bool loadInterfaceParams(const int index =0);
    
    // frustum culling
    void calcFrustumPlane( ci::Vec3f &fNormal, ci::Vec3f& fPoint, float& fDist, const ci::Vec3f& v1, const ci::Vec3f& v2, const ci::Vec3f& v3 );
	void calcNearAndFarClipCoordinates( const ci::Camera& cam );	

protected:
    friend class OculonApp;
    OculonApp* mApp;
    
    std::string mName;
    
    bool        mIsRunning;
    bool        mIsVisible;
    bool        mIsDebug;
    
    bool        mEnableFrustumCulling;
    
    Interface*              mInterface;
    ci::params::InterfaceGl mDebugParams;
    
    bool mDoReset;
    
private:
    // frustum culling
    enum
    { 
        TOP, 
        BOT, 
        LEF, 
        RIG, 
        NEA, 
        FARP,
    
        SIDE_COUNT
    };
    bool        mIsFrustumPlaneCached;
    struct tFrustumPlane
    {
        ci::Vec3f mNormal;
        ci::Vec3f mPoint;
        float mDistance;
    };
    
    tFrustumPlane mCachedFrustumPlane[SIDE_COUNT];
    
    // constants
    static const char* const    kIniLocation;
    static const char* const    kIniExt;
    static const int            kIniDigits;
    
};

#endif // __SCENE_H__
