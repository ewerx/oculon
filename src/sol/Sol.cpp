/*
 *  Sol.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-27.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "AudioInput.h"
#include "Interface.h"
#include "OculonApp.h"
#include "Resources.h"
#include "Sol.h"
#include "TextEntity.h"
#include "TextureManager.h"
#include "cinder/Filesystem.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

// ----------------------------------------------------------------
//
Sol::Sol()
: Scene("sol")
, mIndex(0)
{
    for( int i=0; i < TB_COUNT; ++i )
    {
        mTextBox[i] = new TextEntity(this);
        assert(mTextBox[i] != NULL && "out of memory, how ridiculous");
    }
}

// ----------------------------------------------------------------
//
Sol::~Sol()
{
    for( int i=0; i < TB_COUNT; ++i )
    {
        delete mTextBox[i];
        mTextBox[i] = NULL;
    }
}

// ----------------------------------------------------------------
//
void Sol::setup()
{
    Scene::setup();
    
    mFrameRate = 25.0f;
    mCurrentSource = 0;
    
    mMaskTexture = gl::Texture( loadImage( loadResource( RES_SOLMASK ) ) );
    mDrawMask = true;
    mMultiSource = true;
    
    mPlaybackMode = PLAYBACK_FORWARD;
    mPingPong = false;
    
    setupHud();
    
    initFrames();
    reset();
}

// ----------------------------------------------------------------
//
void Sol::initFrames()
{
    //mTextures = AssetManager::getInstance()->getTextureListFromDir( "/Volumes/cruxpod/Downloads/_images/eit171test/" );
    
    int index = 0;
    //int framesMissing = 0;
    std::string srcDir("/Volumes/cruxpod/Downloads/_images/sdo_20120605/");
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
                bool added = false;
                if( index < mFrames.size() && mFrames[index] != NULL )
                {
                    added = mFrames[index]->init( it->path() );
                    if( !added )
                    {
                        // next frame
                        ++index;
                        
                        // TESTING: limit frames
                        //if( index > 20 )
                            //break;
                    }
                }
                
                if( !added )
                {
                    // file did not belond to the frame
                    // or there are no frames, so make a new frame
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
}

// ----------------------------------------------------------------
//
void Sol::setupInterface()
{
    //mInterface->addParam(CreateBoolParam( "Motion Blur", &mUseMotionBlur )
    //                     .defaultValue(mUseMotionBlur));
    mInterface->addEnum(CreateEnumParam("Source", &mCurrentSource)
                        .maxValue(SolFrame::SOURCE_COUNT)
                        .oscReceiver(mName,"source"));
    
    mInterface->addButton(CreateTriggerParam("Next Source", NULL)
                          .oscReceiver(mName,"nextsource"))->registerCallback( this, &Sol::nextSource );
    
    mInterface->addEnum(CreateEnumParam("PlaybackMode", (int*)(&mPlaybackMode))
                        .maxValue(PLAYBACK_COUNT)
                        .oscReceiver(mName,"playback"));
    
    mInterface->addParam(CreateBoolParam("PingPong", &mPingPong)
                         .oscReceiver(mName,"pingpong"));
    
    mInterface->addParam(CreateFloatParam("Framerate", &mFrameRate)
                         .maxValue(60.0f)
                         .oscReceiver(mName,"framerate"));
    
    mInterface->addParam(CreateBoolParam("Multisource", &mMultiSource)
                         .oscReceiver(mName,"multisource"));
}

// ----------------------------------------------------------------
//
void Sol::setupDebugInterface()
{
    Scene::setupDebugInterface();
    
    mDebugParams.addParam("Draw Mask", &mDrawMask );
    mDebugParams.addParam("Index", &mIndex, "readonly" );
    
    mDebugParams.setOptions("Source", "min=0 max=5");
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
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->resize();
    }
}

// ----------------------------------------------------------------
//
void Sol::update(double dt)
{
    mFrameTime += dt;
    if( mFrameTime > (1.0f/mFrameRate) )
    {
        mFrameTime = 0.0f;
        
        //console() << "[sol] unloading frame " << mIndex << std::endl;
        mFrames[mIndex]->unloadTextures();

        setNextFrame();
    }
    
    // last
    Scene::update(dt);
}

// ----------------------------------------------------------------
//
void Sol::setNextFrame()
{
    switch( mPlaybackMode )
    {
        case PLAYBACK_FORWARD:
            ++mIndex;
            if( mIndex >= mFrames.size() )
            {
                if( mPingPong )
                {
                    mIndex -= 2;
                    mPlaybackMode = PLAYBACK_REVERSE;
                }
                else
                {
                    mIndex = 0;
                }
            }
            break;
            
        case PLAYBACK_REVERSE:
            --mIndex;
            if( mIndex < 0 )
            {
                if( mPingPong )
                {
                    mIndex = 1;
                    mPlaybackMode = PLAYBACK_FORWARD;
                }
                else
                {
                    mIndex = mFrames.size()-1;
                }
            }
            break;
            
        case PLAYBACK_RANDOM:
            mIndex = Rand::randInt( mFrames.size() );
            break;
            
        default:
            break;
    }
}

// ----------------------------------------------------------------
//
void Sol::draw()
{
    gl::pushMatrices();

    //gl::clear(ColorA(1,1,1,0.0f));
    gl::enableAlphaBlending();
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    const float width = mApp->getViewportWidth();
    const float height = mApp->getViewportHeight();
    
    gl::setMatricesWindow( width, height );
    
    if( mFrames.size() > 0 && mIndex < mFrames.size() )
    {
        if( mMultiSource )
        {
            // draw 4 sources in grid
            gl::Texture tex[SolFrame::SOURCE_COUNT];
            Rectf frameRect[SolFrame::SOURCE_COUNT];
            for( int i=0; i < SolFrame::SOURCE_COUNT; ++i )
            {
                tex[i] = mFrames[mIndex]->getTexture( static_cast<SolFrame::eImageSource>(i) );
            }
            
            const float texSize = 512;
            const float drawSize = min( texSize, height/2.5f );
            const float x = (width - drawSize*2)/2.0f;
            const float y = (height - drawSize*2)/2.0f;
            
            // TODO: move to resize
            // 0 1
            // 2 3
            frameRect[0] = Rectf(x, y, x+drawSize, y+drawSize);
            frameRect[1] = Rectf(x+drawSize, y, x+drawSize*2, y+drawSize);
            frameRect[2] = Rectf(x, y+drawSize, x+drawSize, y+drawSize*2);
            frameRect[3] = Rectf(x+drawSize, y+drawSize, x+drawSize*2, y+drawSize*2);
            
            if( tex[0] )
            {
                gl::draw( tex[0], frameRect[0] );
            }
            if( tex[1] )
            {
                gl::draw( tex[1], frameRect[1] );
            }
            if( tex[2] )
            {
                gl::draw( tex[2], frameRect[2] );
            }
            if( tex[3] )
            {
                gl::draw( tex[3], frameRect[3]  );
            }
            
            float shade = 0.75f;
            gl::color( ColorA(shade,shade,shade,1.0f) );
            gl::drawStrokedRect( frameRect[0] );
            gl::drawStrokedRect( frameRect[1] );
            gl::drawStrokedRect( frameRect[2] );
            gl::drawStrokedRect( frameRect[3] );
        }
        else
        {
            // draw one source in the center
            gl::Texture tex = mFrames[mIndex]->getTextureWithSubstitution( static_cast<SolFrame::eImageSource>(mCurrentSource) );
            
            if( tex )
            {
                const float texSize = tex.getWidth();
                const float x = (width - texSize)/2.0f;
                const float y = (height - texSize)/2.0f;
                gl::draw( tex, Rectf(x, y, x+texSize, y+texSize) );
            }
            else
            {
                console() << "[sol] WARNING: frame " << mIndex << " missing textures for all sources!" << std::endl;
            }
            
            // mask
            if( mDrawMask )
            {
                const float maskSize = mMaskTexture.getWidth();
                const float x = (width - maskSize)/2.0f;
                const float y = (height - maskSize)/2.0f;
                gl::draw( mMaskTexture, Rectf( x, y, x+maskSize, y+maskSize ) );
            }
        }
        
    }
    
    drawHud();
    
    gl::popMatrices();
}

// ----------------------------------------------------------------
//
void Sol::drawDebug()
{
    //gl::pushMatrices();
    //gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );

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
            nextSource();
            break;
            
        default:
            handled = false;
            break;
    }
    
    return handled;
}

bool Sol::nextSource()
{
    if( ++mCurrentSource >= SolFrame::SOURCE_COUNT ) 
        mCurrentSource = 0;
    
    return false;
}

#pragma mark - HUD

// ----------------------------------------------------------------
//
void Sol::setupHud()
{
    const float margin = 120.0f;
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

// ----------------------------------------------------------------
//
void Sol::updateHud()
{
    // TOP LEFT
    ostringstream oss1;
    
    // Satellite: SDO = Solar Dynamics Observatory
    // Telescope: AIA = Atmospheric Imaging Assembly
    /*
    format satelliteFormat("SOLAR DYNAMICS OBSERVATORY");
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
     */
}

// ----------------------------------------------------------------
//
void Sol::drawHud()
{
    gl::pushMatrices();
    
    gl::enableAlphaBlending();
    
    CameraOrtho textCam(0.0f, mApp->getViewportWidth(), mApp->getViewportWidth(), 0.0f, 0.0f, 50.f);
    gl::setMatrices(textCam);
    
    for( int i = 0; i < TB_COUNT; ++i )
    {
        mTextBox[i]->draw();
    }
    
    const float width = 200.0f;
    const float height = 100.0f;
    const float space = 10.0f;
    const float left = 20.0f;
    const float top = mApp->getViewportHeight() - 20.0f - (height*2.0f) - space;
    //drawHudWaveformAnalyzer(0.0f,100.0f,mApp->getViewportWidth(),height);
    //drawHudSpectrumAnalyzer(left,top+height+space,width,height);
    
    gl::popMatrices();
}