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
    mAudioInputHandler.setup(this, false);
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
    mHighPassSplit          = 0.5f;
    
    mGroupCols              = 0;
    mGroupRows              = 0;
    
    // particles
    mParticleMode           = PARTICLEMODE_CENTER;
    mParticleDecay          = 0.95f;
    mParticleSpeed          = 2.0f;
    mParticleSpawnTime      = 0.0f;
    mParticleSpawnRate      = 0.25f;
    mParticleSpawnByAudio   = true;
    
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
    mInterface->addParam(CreateColorParam("Color 1", &mColor1(), kMinColor, kMaxColor)
                        .oscReceiver(mName,"color1"));
    mInterface->addParam(CreateColorParam("Color 2", &mColor2(), kMinColor, kMaxColor)
                        .oscReceiver(mName,"color2"));
    mInterface->addParam(CreateColorParam("Color 3", &mColor3(), kMinColor, kMaxColor)
                        .oscReceiver(mName,"color3"));
    
    mAudioInputHandler.setupInterface( mInterface );
    
    // particles
    mInterface->gui()->addColumn();
    mInterface->gui()->addLabel("particles");
    mInterface->addParam(CreateFloatParam( "decay", &mParticleDecay )
                         .minValue(0.9f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "speed", &mParticleSpeed )
                         .maxValue(20.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateFloatParam( "spawnrate", &mParticleSpawnRate )
                         .maxValue(1.0f)
                         .oscReceiver(getName()));
    mInterface->addParam(CreateBoolParam( "spawnaudio", &mParticleSpawnByAudio )
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

}

void Grid::setupDebugInterface()
{
    Scene::setupDebugInterface(); // add all interface params
}

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
    
    mAudioInputHandler.update(dt, mApp->getAudioInput());
    
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
            
            if (row > 0 && mGroupRows > 1 && ((row+0) % mGroupRows != 0))
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
        if (mGroupCols > 0)
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


void Grid::updateParticles(double dt)
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    if (mParticleSpawnByAudio)
    {
        for( AudioInputHandler::FftValues::const_iterator audioIt = mAudioInputHandler.fftValuesBegin();
            audioIt != mAudioInputHandler.fftValuesEnd(); ++audioIt)
        {
            const AudioInputHandler::tFftValue fftValue = (*audioIt);
            float value = fftValue.mValue;
            
            if (value > (1.0f - mParticleSpawnRate))
            {
                mParticles.push_back(spawnParticle());
            }
        }
    }
    else
    {
        mParticleSpawnTime += dt;
        if (mParticleSpawnTime >= mParticleSpawnRate)
        {
            mParticles.push_back(spawnParticle());
            mParticleSpawnTime = 0.0f;
        }
    }
    
    // update
    for( list<tParticle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt )
    {
		partIt->mPos() += partIt->mVel;
        partIt->mVel() *= mParticleDecay;
        partIt->mAlpha() *= mParticleDecay;
        
        if( partIt->mPos().x > windowWidth || partIt->mPos().y > windowHeight || partIt->mPos().x < 0 || partIt->mPos().y < 0 || partIt->mAlpha() <= 0.000001f )
        {
			partIt = mParticles.erase(partIt);
        }
    }
    
}

Grid::tParticle Grid::spawnParticle()
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    const float pixWidth = windowWidth / GRID_WIDTH;
    
    switch (mParticleMode)
    {
        case PARTICLEMODE_CENTER:
        {
            float x = Rand::randInt(GRID_WIDTH) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            speed *= Rand::randBool() ? -1.0f : 1.0f;
            return tParticle( Vec2f( x, (GRID_HEIGHT / 2.0f)*pixWidth ), Vec2f( 0.0f, speed ) );
        }
            
        case PARTICLEMODE_CENTERV:
        {
            float y = Rand::randInt(GRID_HEIGHT) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            speed *= Rand::randBool() ? -1.0f : 1.0f;
            return tParticle( Vec2f( (GRID_WIDTH / 2.0f)*pixWidth, y ), Vec2f( speed, 0.0f ) );
        }
            
        case PARTICLEMODE_RAIN:
        {
            float x = Rand::randInt(GRID_WIDTH) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            bool bottom = Rand::randBool();
            speed *= bottom ? -1.0f : 1.0f;
            return tParticle( Vec2f( x, bottom ? (windowHeight - pixWidth) : 0 ), Vec2f( 0.0f, speed ) );
        }
            
        case PARTICLEMODE_SIDES:
        {
            float y = Rand::randInt(GRID_HEIGHT) * pixWidth;
            float speed = Rand::randFloat( mParticleSpeed/4.0f, mParticleSpeed );
            bool right = Rand::randBool();
            speed *= right ? -1.0f : 1.0f;
            return tParticle( Vec2f( right ? (windowWidth - pixWidth) : 0, y ), Vec2f( speed, 0.0f ) );
        }
    }
}

void Grid::drawParticles()
{
    const float windowWidth = mApp->getViewportWidth();
    const float windowHeight = mApp->getViewportHeight();
    
    const float pixWidth = windowWidth / GRID_WIDTH;
    const float pixHeight = windowHeight / GRID_HEIGHT;
    
    for( list<tParticle>::iterator partIt = mParticles.begin(); partIt != mParticles.end(); ++partIt )
    {
        gl::color( mColor1().r, mColor1().g, mColor1().b, mColor1().a * partIt->mAlpha );
        Vec2f pos = partIt->mPos;
        drawSolidRect( Rectf( pos.x, pos.y, pos.x+pixWidth, pos.y+pixHeight ) );
    }
}


#pragma mark -

bool Grid::setColorScheme()
{
    switch (mColorScheme)
    {
        case COLORSCHEME_RED:
            mColor1 = ColorAf( 0.92f, 0.0f, 0.227f, 1.0f );
            mColor2 = ColorAf( 1.0f, 0.372f, 0.0f, 1.0f );
            mColor3 = ColorAf( 1.0f, 0.643f, 0.0f, 1.0f );
            break;
            
        case COLORSCHEME_BLUE:
            break;
            
        case COLORSCHEME_TURQ:
            break;
            
        case COLORSCHEME_GREEN:
            break;
            
        case COLORSCHEME_YELLOW:
            break;
            
        default:
            break;
    }
    
    return false;
}