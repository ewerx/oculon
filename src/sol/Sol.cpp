/*
 *  Sol.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#include "Sol.h"
#include "TextEntity.h"

#include "OculonApp.h"
#include "AudioInput.h"
#include "Interface.h"
#include "Resources.h"
//#include "AssetManager.h"
#include "TextureManager.h"
#include "cinder/Filesystem.h"

using namespace ci;
using namespace ci::app;

// ----------------------------------------------------------------
//
Sol::Sol()
: Scene("sol")
{
}

// ----------------------------------------------------------------
//
Sol::~Sol()
{
}

// ----------------------------------------------------------------
//
void Sol::setup()
{
    mBufferSize = 10;
    
    mFrameRate = 1.0f;
    mCurrentSource = 0;
    //AssetManager::getInstance()->setup();
    initFrames();
    reset();
}

// ----------------------------------------------------------------
//
void Sol::initFrames()
{
    //mTextures = AssetManager::getInstance()->getTextureListFromDir( "/Volumes/cruxpod/Downloads/_images/eit171test/" );
    
    int index = 0;
    std::string srcDir("/Volumes/cruxpod/Downloads/_images/sdo_20120518/");
    fs::path p(srcDir);
    
    if( !fs::is_directory(p) && !fs::exists(p) )
    {
        console() << "[sol] ERROR: folder \"" << srcDir << " not found" << std::endl; 
    }
    for ( fs::directory_iterator it( p ); it != fs::directory_iterator(); ++it )
    {
        if ( fs::is_regular_file( *it ) )
        {
            string filename = it->path().filename().string();
            if( it->path().extension().string().compare( ".jpg" ) == 0 )
            {
                // frame.init
                // move texture load handling to frame
                // outside this loop: another for loop to pre-load buffersize frames
                // clean up last frame and preload next after each frame change
                //gl::Texture tex = ph::TextureManager::getInstance().load( it->path().string() );
                bool added = false;
                if( index < mFrames.size() && mFrames[index] != NULL )
                {
                    added = mFrames[index]->init( it->path() );
                    if( !added )
                    {
                        // next frame
                        ++index;
                        //if( index > 20 )
                            //break;
                    }
                }
                
                if( !added )
                {
                    if( index > 0 )
                    {
                        //console() << "[sol] Frame " << index-1 << " completed with " << mFrames[index-1]->getImageCount() << " images." << std::endl;
                    }
                    
                    if( index == mFrames.size() )
                    {
                        console() << "[sol] Frame " << index << " created" << std::endl;
                        mFrames.push_back( new SolFrame() );
                        mFrames[index]->init( it->path() );
                    }
                }
            }
        }
    }
    
    //for( int i=0; i < mBufferSize; ++i )
    //{
        //mFrames[i]->loadTextures();
    //}
}

// ----------------------------------------------------------------
//
void Sol::setupInterface()
{
    //mInterface->addParam(CreateBoolParam( "Motion Blur", &mUseMotionBlur )
    //                     .defaultValue(mUseMotionBlur));
    mInterface->addEnum(CreateEnumParam("Source", &mCurrentSource)
                        .maxValue(SolFrame::SOURCE_COUNT));
}

// ----------------------------------------------------------------
//
void Sol::setupDebugInterface()
{
    mDebugParams.addParam("Framerate", &mFrameRate );
    mDebugParams.addParam("Index", &mIndex, "readonly" );
    mDebugParams.setOptions("Source", "max=8");
}

// ----------------------------------------------------------------
//  
void Sol::reset()
{
    mIndex = 0;
    mFrameTime = 0;
}

// ----------------------------------------------------------------
//
void Sol::resize()
{
}

// ----------------------------------------------------------------
//
void Sol::update(double dt)
{
    //AssetManager::getInstance()->update();
    
    mFrameTime += dt;
    if( mFrameTime > (1.0f/mFrameRate) )
    {
        mFrameTime = 0.0f;
        
        console() << "[sol] unloading frame " << mIndex << std::endl;
        mFrames[mIndex]->unloadTextures();
        int nextToBuffer = mIndex + mBufferSize;
        if( nextToBuffer >= mFrames.size() )
        {
            nextToBuffer = nextToBuffer - mFrames.size();
        }
        ++mIndex;
        if( mIndex >= mFrames.size() )
        {
            mIndex = 0;
        }
        
        console() << "[sol] loading frame " << nextToBuffer << std::endl;
        //mFrames[nextToBuffer]->loadTextures();
    }
    
    // last
    Scene::update(dt);
}

// ----------------------------------------------------------------
//
void Sol::draw()
{
    gl::pushMatrices();
    
    const float width = mApp->getViewportWidth();
    const float height = mApp->getViewportHeight();
    
    const float texSize = 512;
    const float x = (width - texSize)/2.0f;
    const float y = (height - texSize)/2.0f;
    
    Rectf rect( x, y, x+texSize, y+texSize );
    
    gl::setMatricesWindow( width, height );
    
    if( mFrames.size() > 0 && mIndex < mFrames.size() )
    {
        gl::Texture tex = mFrames[mIndex]->getTexture((SolFrame::eImageSource)mCurrentSource);
        if( tex != gl::Texture() )
        {
            gl::draw( tex, rect );
        }
        else
        {
            console() << "[sol] texture not ready. index: " << mIndex << " src: " << mCurrentSource << std::endl;
        }
    }
    
    drawHud();
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Sol::drawHud()
{
    gl::pushMatrices();
        
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Sol::drawDebug()
{
    //gl::pushMatrices();
    //gl::setMatricesWindow( getWindowWidth(), getWindowHeight() );

    //gl::popMatrices();
}

// ----------------------------------------------------------------
//
bool Sol::handleKeyDown(const KeyEvent& keyEvent)
{
    bool handled = true;
    
    switch (keyEvent.getCode()) 
    {
        case KeyEvent::KEY_SPACE:
            reset();
            handled = false;
            break;
            
        case KeyEvent::KEY_m:
            if( ++mCurrentSource >= SolFrame::SOURCE_COUNT ) mCurrentSource = 0;
            break;
            
        default:
            handled = false;
            break;
    }
    
    return handled;
}
