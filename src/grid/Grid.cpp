//
//  Grid.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2012-12-28.
//
//


#include "Grid.h"
#include "Interface.h"
#include "OculonApp.h"
#include "Resources.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace std;


Grid::Grid()
: Scene("grid")
{
    //mAudioInputHandler.setup(this, false);
}

Grid::~Grid()
{
}

void Grid::setup()
{
    Scene::setup();
    
    // params
    mGridMode               = GRIDMODE_PARTICLES;
    mMotionBlurEnabled      = false;
    
    mColorScheme            = COLORSCHEME_RED;
    setColorScheme();
    mLowPassSplit           = 0.1f;
    mHighPassSplit          = 1.0f;
    
    mGroupCols              = 0;
    mGroupRows              = 0;
    
    // particles
    mParticleMode           = PARTICLEMODE_CENTER;
    mParticleDecay          = 0.95f;
    mParticleSpeed          = 2.0f;
    mParticleSpawnTime      = 0.0f;
    mParticleSpawnRate      = 0.25f;
    mParticleSpawn          = PARTICLESPAWN_MANUAL;
    mParticleWidth          = 1;
    mParticleHeight         = 1;
    mParticleRandomSize     = true;
    mSpawnAmount            = 3;
    mParticleExpand         = true;
    mMirrorTriggers         = true;
    mSecondaryColorRate      = 0.0f;
    
    mMotionBlurRenderer.setup( mApp->getViewportSize(), boost::bind( &Grid::drawScene, this ) );
    
    reset();
}

void Grid::reset()
{
    for (int i=0; i < GRID_WIDTH; ++i)
    {
        for (int j=0; j < GRID_HEIGHT; ++j)
        {
            mPixels[i][j].mValue = 0.0f;
        }
    }
}

void Grid::setupInterface()
{
    vector<string> modeNames;
#define GRIDMODE_ENTRY( nam, enm ) \
modeNames.push_back(nam);
    GRIDMODE_TUPLE
#undef  GRIDMODE_ENTRY
    mInterface->addEnum(CreateEnumParam( "Mode", (int*)(&mGridMode) )
                        .maxValue(GRIDMODE_COUNT)
                        .oscReceiver(getName(), "gridmode")
                        .isVertical(), modeNames);
    
    mInterface->addParam(CreateIntParam( "group_cols", &mGroupCols )
                         .maxValue(GRID_WIDTH/2)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "group_rows", &mGroupRows )
                         .maxValue(GRID_HEIGHT/2)
                         .oscReceiver(getName()));
    
    mInterface->addParam(CreateBoolParam( "motion_blur", &mMotionBlurEnabled )
                         .oscReceiver(getName()));
    
    // colors
    mInterface->gui()->addColumn();
    vector<string> colorSchemeNames;
#define COLORSCHEME_ENTRY( nam, enm ) \
colorSchemeNames.push_back(nam);
    COLORSCHEME_TUPLE
#undef  COLORSCHEME_ENTRY
    mInterface->addEnum(CreateEnumParam( "Color Scheme", (int*)(&mColorScheme) )
                        .maxValue(COLORSCHEME_COUNT)
                        .oscReceiver(getName(), "colorscheme")
                        .isVertical(), colorSchemeNames)->registerCallback( this, &Grid::setColorScheme );
    mInterface->addParam(CreateFloatParam( "lowpass", &mLowPassSplit )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "highpass", &mHighPassSplit )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "dual_color", &mSecondaryColorRate )
                         .oscReceiver(getName())
                         .midiInput(2, 1, 20));
//    mInterface->addParam(CreateColorParam("Color 1", &mColor1(), kMinColor, kMaxColor)
//                        .oscReceiver(mName,"color1"));
    mInterface->addParam(CreateFloatParam( "color1_r", &(mColor1().r) )
                         .oscReceiver(getName())
                         .midiInput(1,2,0));
    mInterface->addParam(CreateFloatParam( "color1_g", &(mColor1().g) )
                         .oscReceiver(getName())
                         .midiInput(1,2,1));
    mInterface->addParam(CreateFloatParam( "color1_b", &(mColor1().b) )
                         .oscReceiver(getName())
                         .midiInput(1,2,2));
    mInterface->addParam(CreateFloatParam( "color1_a", &(mColor1().a) )
                         .oscReceiver(getName())
                         .midiInput(1,2,3));

//    mInterface->addParam(CreateColorParam("Color 2", &mColor2(), kMinColor, kMaxColor)
//                        .oscReceiver(mName,"color2"));
    mInterface->addParam(CreateFloatParam( "color2_r", &(mColor2().r) )
                         .oscReceiver(getName())
                         .midiInput(1,2,4));
    mInterface->addParam(CreateFloatParam( "color2_g", &(mColor2().g) )
                         .oscReceiver(getName())
                         .midiInput(1,2,5));
    mInterface->addParam(CreateFloatParam( "color2_b", &(mColor2().b) )
                         .oscReceiver(getName())
                         .midiInput(1,2,6));
    mInterface->addParam(CreateFloatParam( "color2_a", &(mColor2().a) )
                         .oscReceiver(getName())
                         .midiInput(1,2,7));
//    mInterface->addParam(CreateColorParam("Color 3", &mColor3(), kMinColor, kMaxColor)
//                        .oscReceiver(mName,"color3"));
    
    //mAudioInputHandler.setupInterface( mInterface );
    
    // particles
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("particles");
    mInterface->addParam(CreateFloatParam( "decay", &mParticleDecay )
                         .minValue(0.9f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 16));
    mInterface->addParam(CreateFloatParam( "speed", &mParticleSpeed )
                         .maxValue(20.0f)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 17));
    mInterface->addParam(CreateFloatParam( "spawnrate", &mParticleSpawnRate )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    vector<string> particleModeNames;
#define PARTICLEMODE_ENTRY( nam, enm ) \
particleModeNames.push_back(nam);
    PARTICLEMODE_TUPLE
#undef  PARTICLEMODE_ENTRY
    mInterface->addEnum(CreateEnumParam( "particle_mode", (int*)(&mParticleMode) )
                        .maxValue(PARTICLEMODE_COUNT)
                        .oscReceiver(getName())
                        .isVertical(), particleModeNames);

    vector<string> particleSpawnNames;
#define PARTICLESPAWN_ENTRY( nam, enm ) \
particleSpawnNames.push_back(nam);
    PARTICLESPAWN_TUPLE
#undef  PARTICLESPAWN_ENTRY
    mInterface->addEnum(CreateEnumParam( "particle_spawn", (int*)(&mParticleSpawn) )
                        .maxValue(PARTICLESPAWN_COUNT)
                        .oscReceiver(getName())
                        .isVertical(), particleSpawnNames);
    
    mInterface->addParam(CreateIntParam( "particle_width", &mParticleWidth )
                         .minValue(1)
                         .maxValue(10)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateIntParam( "particle_height", &mParticleHeight )
                         .minValue(1)
                         .maxValue(GRID_HEIGHT)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "particle_randomsize", &mParticleRandomSize )
                         .oscReceiver(getName())
                         .midiInput(1, 2, 64));
    mInterface->addParam(CreateIntParam( "spawn_amt", &mSpawnAmount )
                         .minValue(1)
                         .maxValue(100)
                         .oscReceiver(getName())
                         .midiInput(1, 2, 19));
    mInterface->addButton(CreateTriggerParam("spawn_center", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 43))->registerCallback( boost::bind( &Grid::triggerSpawn, this, PARTICLEMODE_COUNT) );
    mInterface->addButton(CreateTriggerParam("spawn_strips", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 44))->registerCallback( boost::bind( &Grid::triggerSpawn, this, PARTICLEMODE_STRIPS) );
    mInterface->addButton(CreateTriggerParam("spawn_spread", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 42))->registerCallback( boost::bind( &Grid::triggerSpawn, this, PARTICLEMODE_SPREAD) );
    mInterface->addButton(CreateTriggerParam("spawn_sides", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 45))->registerCallback( boost::bind( &Grid::triggerSpawn, this, PARTICLEMODE_SIDES) );
    
    mInterface->gui()->addColumn();
    mInterface->addParam(CreateBoolParam( "particle_expand", &mParticleExpand )
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "mirror", &mMirrorTriggers )
                         .oscReceiver(getName())
                         .midiInput(1, 2, 64));
    mInterface->addButton(CreateTriggerParam("trigger_box0", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 37))->registerCallback( boost::bind( &Grid::triggerBox, this, 0) );
    mInterface->addButton(CreateTriggerParam("trigger_box1", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 39))->registerCallback( boost::bind( &Grid::triggerBox, this, 1) );
    mInterface->addButton(CreateTriggerParam("trigger_box2", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 41))->registerCallback( boost::bind( &Grid::triggerBox, this, 2) );
    mInterface->addButton(CreateTriggerParam("trigger_box3", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 47))->registerCallback( boost::bind( &Grid::triggerBox, this, 4) );
    mInterface->addButton(CreateTriggerParam("trigger_box4", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 49))->registerCallback( boost::bind( &Grid::triggerBox, this, 5) );
    mInterface->addButton(CreateTriggerParam("trigger_box5", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 51))->registerCallback( boost::bind( &Grid::triggerBox, this, 6) );
    mInterface->addButton(CreateTriggerParam("trigger_box6", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 36))->registerCallback( boost::bind( &Grid::triggerBox, this, 7) );
    mInterface->addButton(CreateTriggerParam("trigger_box7", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 38))->registerCallback( boost::bind( &Grid::triggerBox, this, 8) );
    mInterface->addButton(CreateTriggerParam("trigger_box8", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 40))->registerCallback( boost::bind( &Grid::triggerBox, this, 9) );
    mInterface->addButton(CreateTriggerParam("trigger_box9", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 46))->registerCallback( boost::bind( &Grid::triggerBox, this, 11) );
    mInterface->addButton(CreateTriggerParam("trigger_box10", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 48))->registerCallback( boost::bind( &Grid::triggerBox, this, 12) );
    mInterface->addButton(CreateTriggerParam("trigger_box11", NULL)
                          .oscReceiver(getName())
                          .midiInput(2, 1, 50))->registerCallback( boost::bind( &Grid::triggerBox, this, 13) );
    

}

//void Grid::setupDebugInterface()
//{
//    Scene::setupDebugInterface(); // add all interface params
//}

void Grid::update(double dt)
{
    Scene::update(dt);
    
    // updates
    switch (mGridMode)
    {
        case GRIDMODE_MOTION:
            break;
            
        case GRIDMODE_PIXELS:
            updatePixels();
            break;
        
        case GRIDMODE_PARTICLES:
            updateParticles(dt);
            break;
            
        default:
            break;
    }
    
    // pre-draw
	gl::disableDepthRead();
	gl::disableDepthWrite();
    
    //mAudioInputHandler.update(dt, mApp->getAudioInput());
    
    gl::enableAlphaBlending();
    if (mMotionBlurEnabled)
    {
        mMotionBlurRenderer.preDraw();
    }
}

void Grid::draw()
{
	gl::enableAlphaBlending();

    gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::pushMatrices();

    if (mMotionBlurEnabled)
    {
        mMotionBlurRenderer.draw();
    }
    else
    {
        gl::setMatricesWindow( mApp->getViewportWidth(), mApp->getViewportHeight() );
        drawScene();
    }

	// restore camera and render states
    gl::disable( GL_TEXTURE_2D );
	gl::popMatrices();

	gl::disableDepthWrite();
	gl::disableDepthRead();

	gl::disableAlphaBlending();
}

void Grid::drawScene()
{
    switch (mGridMode)
    {
        case GRIDMODE_MOTION:
            break;
            
        case GRIDMODE_PIXELS:
            drawPixels();
            break;
            
        case GRIDMODE_PARTICLES:
            drawParticles();
            break;
            
        default:
            break;
    }
}

void Grid::drawDebug()
{
    gl::enable( GL_TEXTURE_2D );
    gl::setMatricesWindow( getWindowSize() );
    
    // grid
    gl::setMatricesWindow( mApp->getViewportSize() );
    gl::disable( GL_TEXTURE_2D );
    gl::color(1.0f,1.0f,1.0f,1.0f);
    int x = 0;
    int y = 0;
    int pixWidth = mApp->getViewportWidth() / GRID_WIDTH;
    int pixHeight = mApp->getViewportHeight() / GRID_HEIGHT;
   
    for (int i = 0; i < GRID_HEIGHT; ++i)
    {
        x = 0;
        for (int j = 0; j < GRID_WIDTH; ++j)
        {
            drawStrokedRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            x += pixWidth;
        }
        y += pixHeight;
    }
}

#pragma mark - Pixels

void Grid::updatePixels()
{
    AudioInputHandler::FftValues::const_iterator audioIt = mAudioInputHandler.fftValuesBegin();
    //AudioInputHandler::FftValues& fftValues = mAudioInputHandler.getFftValues();
    
    for (int row = 0; row < GRID_HEIGHT; ++row)
    {
        for (int col = 0; col < GRID_WIDTH; ++col)
        {
            if (audioIt == mAudioInputHandler.fftValuesEnd())
            {
                audioIt = mAudioInputHandler.fftValuesBegin();
            }
            
            const AudioInputHandler::tFftValue fftValue = (*audioIt);
            
            float value = fftValue.mValue;
            
            if (row > 0 && mGroupRows > 1 && ((row+1) % mGroupRows != 0))
            {
                value = mPixels[row-1][col].mValue;
            }
            
            mPixels[row][col].mValue = value;
            float freq = (float)(fftValue.mBandIndex) / (float)(mApp->getAudioInput().getFft()->getBinSize());
            mPixels[row][col].mFreq = freq;

            if (mGroupCols == 0 || ((col+1) % mGroupCols == 0))
            {
                audioIt++;
            }
        }
        if (mGroupRows > 0)
        {
            audioIt++;
        }
    }
}

void Grid::drawPixels()
{
    // dimensions
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    const float pixWidth = windowWidth / GRID_WIDTH;
    const float pixHeight = windowHeight / GRID_HEIGHT;
    
    // draw grid
    gl::setMatricesWindow( mApp->getViewportSize() );
    gl::disable( GL_TEXTURE_2D );
    gl::color( ColorAf::white() );
    float x = 0;
    float y = 0;
    
    for (int row = 0; row < GRID_HEIGHT; ++row)
    {
        x = 0;
        for (int col = 0; col < GRID_WIDTH; ++col)
        {
            const float value = mPixels[row][col].mValue;
            const float freq = mPixels[row][col].mFreq;

            if (freq < mLowPassSplit) {
                float m = 1.0f - freq/mLowPassSplit;
                gl::color( mColor1().r * m, mColor1().g * m, mColor1().b * m, mColor1().a * value );
            } else if (mHighPassSplit > mLowPassSplit && freq < mHighPassSplit) {
                float m = freq/(mHighPassSplit-mLowPassSplit);
                gl::color( mColor2().r * m, mColor2().g * m, mColor2().b * m, mColor2().a * value );
            } else {
                float m = (1.0f - freq)/(1.0f - mHighPassSplit);
                gl::color( mColor3().r * m, mColor3().g * m, mColor3().b * m, mColor3().a * value );
            }
            drawSolidRect( Rectf( x, y, x+pixWidth, y+pixHeight ) );
            
            x += pixWidth;
        }
        y += pixHeight;
    }
}

#pragma mark - Particles

bool Grid::triggerSpawn(int type)
{
    for (int i = 0; i < mSpawnAmount; ++i )
    {
        mParticles.push_back(spawnParticle(type));
    }
    
    return false;
}

void Grid::updateParticles(double dt)
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    if (mParticleSpawn == PARTICLESPAWN_AUDIO)
    {
        for( AudioInputHandler::FftValues::const_iterator audioIt = mAudioInputHandler.fftValuesBegin();
            audioIt != mAudioInputHandler.fftValuesEnd(); ++audioIt)
        {
            const AudioInputHandler::tFftValue fftValue = (*audioIt);
            float value = fftValue.mValue;
            
            if (value > (1.0f - mParticleSpawnRate) && mParticles.size() < 600)
            {
                mParticles.push_back(spawnParticle());
            }
        }
    }
    else if (mParticleSpawn == PARTICLESPAWN_TIME)
    {
        mParticleSpawnTime += dt;
        if (mParticleSpawnTime >= mParticleSpawnRate && mParticles.size() < 600)
        {
            mParticles.push_back(spawnParticle());
            mParticleSpawnTime = 0.0f;
        }
    }
//    else if (mParticleMode == PARTICLEMODE_FIXED && mParticles.size() < (GRID_WIDTH * GRID_HEIGHT))
//    {
//        // spwan them all once
//        
//    }
    
    // update
//    if (mParticleMode != PARTICLEMODE_FIXED)
    {
        for( list<tParticle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt )
        {
            partIt->mPos() += partIt->mVel;
            //partIt->mVel() *= mParticleDecay;
            
            
            partIt->mAlpha() *= mParticleDecay;
            
            
            if (mParticles.size() > 300) {
                // too many! double decay!
                partIt->mAlpha() *= mParticleDecay;
            }
            
            
            if( partIt->mPos().x > windowWidth || partIt->mPos().y > windowHeight || partIt->mPos().x < 0 || partIt->mPos().y < 0 || partIt->mAlpha() <= 0.000001f )
            {
                partIt = mParticles.erase(partIt);
            }
        }
    }
}

Grid::tParticle Grid::spawnParticle(int type)
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    const float pixWidth = windowWidth / GRID_WIDTH;
    
    int width = mParticleRandomSize ? Rand::randInt(1,mParticleWidth) : mParticleWidth;
    int height = mParticleRandomSize ? Rand::randInt(1,mParticleHeight) : mParticleHeight;
    
    tParticle p;
    
    if (type >= PARTICLEMODE_COUNT)
    {
        type = mParticleMode;
    }
    
    switch (type)
    {
//        case PARTICLEMODE_FIXED:
//        {
//            int xpos = mParticles.size() % 35;
//            float x = xpos * pixWidth;
//            int ypos = mParticles.size() / 35;
//            float y = ypos * pixWidth;
//            p = tParticle( Vec2f( x, y ), Vec2f( 0.0f, 0.0f ), width, height );
//            break;
//        }
            
        case PARTICLEMODE_CENTER:
        {
            float x = Rand::randInt(GRID_WIDTH) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            speed *= Rand::randBool() ? -1.0f : 1.0f;
            p = tParticle( Vec2f( x, (GRID_HEIGHT / 2.0f)*pixWidth ), Vec2f( 0.0f, speed ), width, height );
            break;
        }
            
        case PARTICLEMODE_CENTERV:
        {
            float y = Rand::randInt(GRID_HEIGHT) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            speed *= Rand::randBool() ? -1.0f : 1.0f;
            p = tParticle( Vec2f( (GRID_WIDTH / 2.0f)*pixWidth - pixWidth, y ), Vec2f( speed, 0.0f ), width, height );
            break;
        }
            
        case PARTICLEMODE_RAIN:
        {
            float x = Rand::randInt(GRID_WIDTH) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            bool bottom = Rand::randBool();
            speed *= bottom ? -1.0f : 1.0f;
            p = tParticle( Vec2f( x, bottom ? (windowHeight - pixWidth) : 0 ), Vec2f( 0.0f, speed ), width, height );
            break;
        }
            
        case PARTICLEMODE_SIDES:
        {
            float y = Rand::randInt(GRID_HEIGHT) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            bool right = Rand::randBool();
            speed *= right ? -1.0f : 1.0f;
            p = tParticle( Vec2f( right ? (windowWidth - pixWidth) : 0, y ), Vec2f( speed, 0.0f ), width, height );
            break;
        }
            
        case PARTICLEMODE_SPREAD:
        {
            float x = Rand::randInt(GRID_WIDTH) * pixWidth;
            float y = Rand::randInt(GRID_HEIGHT) * pixWidth;
            float speedx = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed ) * (Rand::randBool()  ? -1.0f : 1.0f);
            float speedy = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed ) * (Rand::randBool()  ? -1.0f : 1.0f);
            p = tParticle( Vec2f( x, y ), Vec2f( speedx, speedy ), width, height );
            break;
        }
            
        case PARTICLEMODE_STRIPS:
        {
            float x = Rand::randInt(GRID_WIDTH) * pixWidth;
            float y = Rand::randInt(GRID_HEIGHT) * pixWidth;
            float speedx = 0.0f;
            float speedy = 0.0f;
            bool tall = Rand::randBool();
            if( tall )
            {
                width = 1;
                height = math<int>::max(2,height);
                speedy = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed ) * (Rand::randBool()  ? -1.0f : 1.0f);
            }
            else
            {
                height = 1;
                width = math<int>::max(2,width);
                speedx = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed ) * (Rand::randBool()  ? -1.0f : 1.0f);
            }
            
            p = tParticle( Vec2f( x, y ), Vec2f( speedx, speedy ), width, height );
            break;
        }
            
        default:
            break;
    }
    
    //timeline().apply( &p.mVel, Vec2f::zero(), mAudioInputHandler.mFalloffTime, mAudioInputHandler.getFalloffFunction() );
    //timeline().apply( &p.mAlpha, 0.0f, mAudioInputHandler.mFalloffTime, mAudioInputHandler.getFalloffFunction() );
    
    return p;
}

void Grid::drawParticles()
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    const float pixWidth = windowWidth / GRID_WIDTH;
    const float pixHeight = windowHeight / GRID_HEIGHT;
    
    for( list<tParticle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt )
    {
        if (Rand::randFloat() < mSecondaryColorRate)
        {
            gl::color( mColor2().r, mColor2().g, mColor2().b, mColor2().a * partIt->mAlpha );
        }
        else
        {
            gl::color( mColor1().r, mColor1().g, mColor1().b, mColor1().a * partIt->mAlpha );
        }
        Vec2f pos = partIt->mPos;
        
        if( partIt->mExpand )
        {
            float x = pos.x + (5-partIt->mWidth)*pixWidth/2.0f;
            float y = pos.y + (5-partIt->mHeight)*pixWidth/2.0f;
            drawSolidRect( Rectf( x, y, x+pixWidth*partIt->mWidth, y+pixHeight*partIt->mHeight ) );
        }
        else
        {
            drawSolidRect( Rectf( pos.x, pos.y, pos.x+pixWidth*partIt->mWidth, pos.y+pixHeight*partIt->mHeight ) );
        }
    }
}

#pragma mark - Snakes
//
//void Grid::updateSnakes(double dt)
//{
//    
//}
//
//void Grid::drawSnakes()
//{
//    for( vector<tParticle>::iterator partIt = mVSnakes.begin(); partIt != mVSnakes.end(); ++partIt )
//    {
//        gl::color( mColor1().r, mColor1().g, mColor1().b, mColor1().a * partIt->mAlpha );
//        Vec2f pos = partIt->mPos;
//        Vec2f vel = partIt->mVel;
//        Vec2f tail = pos;
//        tail.y = ( vel.y > 0.0f ) ? tail.y - mSnakeLength : tail.y + mSnakeLength;
//        
//        Rectf rect = ( vel.y > 0.0f ) ? Rectf( tail, pos ) : Rectf( pos, tail );
//        rect.inflate( Vec2f( thickness, 0.0f ) );
//        
//        if (vel.y > 0)
//        {
//            
//        }
//        else
//        {
//            
//        }
//    }
//}
//
//bool Grid::resetSnakes()
//{
//    const float pixWidth = windowWidth / GRID_WIDTH;
//    float yPer = (mApp->getViewportHeight() / (mNumHSnakes * pixWidth)) / (mNumHSnakes + 1);
//    float y = yPer;
//    for (int i=0; i < mNumHSnakes; ++i)
//    {
//        bool left = Rand::randBool();
//        float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
//        tParticle snake( Vec2f( left ? 0 : mApp->getViewportWidth(), y ), Vec2f( left ? speed : -speed, 0.0f ), mSnakeLength, 1 );
//        mHSnakes.push_back(snake);
//        y += yPer;
//    }
//    
//    float xPer = (mApp->getViewportWidth() / (mNumVSnakes * pixWidth)) / (mNumVSnakes + 1);
//    float x = xPer;
//    for (int i=0; i < mNumVSnakes; ++i)
//    {
//        bool top = Rand::randBool();
//        float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
//        tParticle snake( Vec2f( x, y ), Vec2f( 0.0f, top ? speed : -speed ), 1, mSnakeLength );
//        mVSnakes.push_back(snake);
//        x += xPer;
//    }
//}

#pragma mark -

bool Grid::setColorScheme()
{
    switch (mColorScheme)
    {
        case COLORSCHEME_RED:
            mColor1 = ColorAf( 0.92f, 0.0f, 0.227f, 1.0f );
            mColor2 = ColorAf( 1.0f, 0.0f, 0.0f, 1.0f );
            mColor3 = ColorAf( 1.0f, 0.0f, 0.2f, 1.0f );
            break;
            
        case COLORSCHEME_BLUE:
            mColor1 = ColorAf( 0.0f, 0.0f, 1.0f, 1.0f );
            mColor2 = ColorAf( 0.2f, 0.0f, 1.0f, 1.0f );
            mColor3 = ColorAf( 0.1f, 0.1f, 1.0f, 1.0f );
            break;
            
        case COLORSCHEME_TURQ:
            mColor1 = ColorAf( 0.0f, 0.0f, 1.0f, 1.0f );
            mColor2 = ColorAf( 0.0f, 1.f, 1.0f, 1.0f );
            mColor3 = ColorAf( 0.0f, 0.8f, 0.8f, 1.0f );
            break;
            
        case COLORSCHEME_GREEN:
            mColor1 = ColorAf( 0.0f, 0.0f, 1.0f, 1.0f );
            mColor2 = ColorAf( 0.0f, 0.2f, 1.0f, 1.0f );
            mColor3 = ColorAf( 0.0f, 0.3f, 1.0f, 1.0f );
            break;
            
        case COLORSCHEME_YELLOW:
            mColor1 = ColorAf( 1.0f, 0.2f, 0.0f, 1.0f );
            mColor2 = ColorAf( 1.0f, 0.3f, 0.0f, 1.0f );
            mColor3 = ColorAf( 0.9f, 0.4f, 0.0f, 1.0f );
            break;
            
        default:
            break;
    }
    
    return false;
}

bool Grid::triggerBox(int pos)
{
    triggerOneBox(pos);
    
    if (mMirrorTriggers)
    {
        switch(pos)
        {
            case 0:
                triggerOneBox(6);
                break;
                
            case 1:
                triggerOneBox(5);
                break;
                
            case 2:
                triggerOneBox(4);
                break;
                
            case 7:
                triggerOneBox(13);
                break;
                
            case 8:
                triggerOneBox(12);
                break;
                
            case 9:
                triggerOneBox(11);
                break;
                
        }
    }
    
    return false;
}

void Grid::triggerOneBox(int pos)
{
    const float pixWidth = mApp->getViewportWidth() / GRID_WIDTH;
    
    int width = 5;//mParticleRandomSize ? Rand::randInt(1,mParticleWidth) : mParticleWidth;
    int height = 5;//mParticleRandomSize ? Rand::randInt(1,mParticleHeight) : mParticleHeight;
    
    float y = 0;
    if (pos > 6)
    {
        y = pixWidth * 5;
        pos -= 7;
    }
    
    float x = pixWidth * width * pos;
    
    tParticle p = tParticle( Vec2f( x, y ), Vec2f( 0.0f, 0.0f ), 0, 0 );
    if (mParticleExpand)
    {
        timeline().apply( &p.mWidth, width, mAudioInputHandler.mFalloffTime, mAudioInputHandler.getFalloffFunction() );
        timeline().apply( &p.mHeight, height, mAudioInputHandler.mFalloffTime, mAudioInputHandler.getFalloffFunction() );
        p.mExpand = true;
    }
    mParticles.push_back( p );
}