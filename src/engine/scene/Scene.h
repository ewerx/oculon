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
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "cinder/app/KeyEvent.h"
#include "cinder/app/MouseEvent.h"
#include "cinderSyphon.h"
#include "AudioInputHandler.h"

// fwd decl
class OculonApp;
class Interface;


class Scene
{
public:
    Scene(const std::string& name);
    virtual ~Scene();
    
    void init(OculonApp* app);
    virtual void setup();
    virtual void reset() {}
    virtual void shutdown() {}
    virtual void resize();
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
    
    virtual const ci::Camera& getCamera();
    virtual AudioInputHandler& getAudioInputHandler();
    
    void setupInterfaces();
    void drawToFbo();
    void publishToSyphon();
    ci::gl::Fbo& getFbo()                   { return mFbo; }
    ci::gl::Texture getFboTexture()         { return mFbo.getTexture(); }
    
    OculonApp* getApp() const               { return mApp; }
    const std::string& getName() const      { return mName; }
    
    Interface* getInterface()                       { return mInterface; }
//    ci::params::InterfaceGl* getDebugInterface()    { return &mDebugParams; }
    
    bool isSetup() const            { return mIsSetup; }
    bool isRunning() const          { return mIsRunning; }
    bool isVisible() const          { return mIsVisible; }
    bool isDebug() const            { return mIsDebug; }
    int getLayerIndex() const       { return mLayerIndex; }
    
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
    
    virtual void setupInterface() {}
    //virtual void setupDebugInterface();
    void setupFbo();
    
    bool saveInterfaceParams();
    bool loadInterfaceParams(const int index =0);
    void setupLoadParamsInterface();
    bool showLoadParamsInterface();
    bool hideLoadParamsInterface();
    
    // frustum culling
    void calcFrustumPlane( ci::Vec3f &fNormal, ci::Vec3f& fPoint, float& fDist, const ci::Vec3f& v1, const ci::Vec3f& v2, const ci::Vec3f& v3 );
	void calcNearAndFarClipCoordinates( const ci::Camera& cam );
    
    // helpers
    ci::gl::GlslProg loadFragShader( const std::string& filename );
    ci::gl::GlslProg loadVertAndFragShaders( const std::string& vertShader, const std::string& fragShader );

protected:
    friend class OculonApp;
    OculonApp* mApp;
    
    std::string mName;
    
    bool        mIsSetup;
    bool        mIsRunning;
    bool        mIsVisible;
    bool        mIsDebug;
    
    bool        mEnableFrustumCulling;
    
    float       mBackgroundAlpha;
    
    Interface*              mInterface;
    Interface*              mParamsInterface;
    Interface*              mLoadParamsInterface;
//    ci::params::InterfaceGl mDebugParams;
    
    bool mDoReset;
    
private:
    // ini files
    std::vector<std::string> mIniFilenames;
    // fbo
    ci::gl::Fbo             mFbo;
    syphonServer            mSyphon;
    int                     mLayerIndex;
    
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
