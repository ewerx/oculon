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

using namespace ci;


class Scene
{
public:
    Scene();
    virtual ~Scene();
    
    void init(OculonApp* app);
    
    virtual void reset() {}
    virtual void shutdown() {}
    virtual void resize() {}
    virtual void update(double dt);
    virtual void draw() {}
    virtual void drawDebug() {}
    
    virtual bool handleKeyDown( const ci::app::KeyEvent& keyEvent )         { return false; }
    virtual void handleMouseDown( const ci::app::MouseEvent& mouseEvent )   { return; }
	virtual void handleMouseUp( const ci::app::MouseEvent& event)           { return; }
	virtual void handleMouseDrag( const ci::app::MouseEvent& event )        { return; }
    
    OculonApp* getApp() { return mApp; }
    const Camera& getCamera();
    
    bool isActive() const           { return mIsActive; }
    bool isVisible() const          { return mIsVisible; }
    
    virtual void setActive(bool active)     { mIsActive = active; }
    virtual void setVisible(bool visible)   { mIsVisible = visible; }
    
    void toggleActiveVisible()      { mIsActive = !mIsActive; mIsVisible = !mIsVisible; }
    
    // frustum culling
    bool isFrustumCullingEnabled()  { return mEnableFrustumCulling; }
    void setFrustumCulling( bool enabled ) { mEnableFrustumCulling = enabled; }
    bool isPointInFrustum( const Vec3f &loc );
	bool isSphereInFrustum( const Vec3f &loc, float radius );
	bool isBoxInFrustum( const Vec3f &loc, const Vec3f &size );
    
protected:
    
    virtual void setup() {}
    virtual void setupParams(ci::params::InterfaceGl& params) {}
    
    // frustum culling
    void calcFrustumPlane( Vec3f &fNormal, Vec3f& fPoint, float& fDist, const Vec3f& v1, const Vec3f& v2, const Vec3f& v3 );
	void calcNearAndFarClipCoordinates( const Camera& cam );	

protected:
    OculonApp* mApp;//TODO: fix this dependency
    
    bool        mIsActive;
    bool        mIsVisible;
    
    
    bool        mEnableFrustumCulling;
    
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
        Vec3f mNormal;
        Vec3f mPoint;
        float mDistance;
    };
    
    tFrustumPlane mCachedFrustumPlane[SIDE_COUNT];
    
};

#endif // __SCENE_H__
