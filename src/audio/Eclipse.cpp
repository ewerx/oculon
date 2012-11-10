/*
 *  Eclipse.cpp
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */


#include "Eclipse.h"
#include "Utils.h"
#include "Scene.h"
#include "OculonApp.h"
#include "Interface.h"

#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Rand.h"
#include "cinder/Easing.h"

using namespace ci;
using namespace std;


//
// Eclipse
// 

Eclipse::Eclipse(Scene* scene)
: SubScene(scene,"eclipse")
{
    mDim = 168; // cube dimension
    mXOffset = 8;
    mYOffset = 130;
    
    mRectsInRow[0] = ROW1_RECTS;
    mRectsInRow[1] = ROW2_RECTS;
    mRectsInRow[2] = ROW3_RECTS;
    
    mLinesInRow[0] = ROW1_RECTS+1;
    mLinesInRow[1] = ROW1_RECTS+1;
    mLinesInRow[2] = ROW2_RECTS+1;
    mLinesInRow[3] = ROW3_RECTS+1;
    
    mMinColForRow[0] = mMinColForRow[1] = 0;
    mMinColForRow[2] = 1;
    mMinColForRow[3] = 2;
    
    mMaxColForRow[0] = mMaxColForRow[1] = 6;
    mMaxColForRow[2] = 5;
    mMaxColForRow[3] = 4;
    
    mXMin[0] = mXMin[1] = mXOffset;
    mXMin[2] = mXMin[1] + mDim;
    mXMin[3] = mXMin[2] + mDim;
    
    mXMax[0] = mXMax[1] = mXOffset + mDim*6;
    mXMax[2] = mXMax[1] - mDim;
    mXMax[3] = mXMax[2] - mDim;
    
    for( int i=0; i < NUM_VLINES; ++i )
    {
        mYMin[i] = mYOffset;
    }
    mYMax[0] = mYOffset + mDim;
    mYMax[1] = mYMax[0] + mDim;
    mYMax[2] = mYMax[3] = mYMax[4] = mYMax[1] + mDim;
    mYMax[5] = mYMax[1];
    mYMax[6] = mYMax[0];
}

Eclipse::~Eclipse()
{
}

void Eclipse::setup()
{
    mShowTestBackground = false;
    mTestBackground = ci::loadImage( ci::loadFile( "/Users/ehsan/Downloads/eclipse_mapping_pattern.jpg" ) );
    
    mMode = MODE_SNAKES;
    mInverse = false;
    mDrawInnerLines = false;
    mMaxThickness = mDim/2.0f;
    mRandomizeFaces = false;
    mRandomizeSignal = false;
    mUseFftData = true;
    mSignalScale = 4.0f;
    
    mNumSnakes = 4;
    mSnakeLength = mDim/1.2f;
    mMinSnakeSpeed = 15.0f;
    mMaxSnakeSpeed = 15.0f;
    mSnakeDirMode = SNAKEDIR_ALTERNATE;
    mSnakeRounded = false;
    mSnakeRandomSpeed = false;
    mSnakeSpeedMultiplier = 1.0f;
    
    mColor = Colorf::white();
    
    reset();
}

void Eclipse::setupInterface()
{
    SubScene::setupInterface();
    
    Interface* interface = mParentScene->getInterface();
    interface->addParam(CreateBoolParam("Test Pattern", &mShowTestBackground)
                        .oscReceiver(mName,"testpattern"));
    interface->addEnum(CreateEnumParam("Draw Mode", (int*)&mMode)
                       .maxValue(MODE_COUNT)
                       .oscReceiver(mName,"drawmode")
                       .isVertical());
    interface->addParam(CreateBoolParam("Inverse", &mInverse)
                        .oscReceiver(mName,"inverse"));
    interface->addParam(CreateBoolParam("Draw Inner Lines", &mDrawInnerLines)
                        .oscReceiver(mName,"drawinnerlines"));
    interface->addParam(CreateBoolParam("Randomize Faces", &mRandomizeFaces)
                        .oscReceiver(mName,"randomfaces"));
    interface->addParam(CreateBoolParam("Randomize Signal", &mRandomizeSignal)
                        .oscReceiver(mName,"randomsignal"));
    interface->addParam(CreateBoolParam("Use FFT", &mUseFftData)
                        .oscReceiver(mName,"usefft"));
    interface->addParam(CreateFloatParam("Max Thickness", &mMaxThickness)
                        .minValue(2.0f)
                        .maxValue((float)mDim/2.0f)
                        .oscReceiver(mName,"maxthickness"));
    interface->addParam(CreateFloatParam("Signal Scale", &mSignalScale)
                        .minValue(1.0f)
                        .maxValue(10.0f)
                        .oscReceiver(mName,"signalscale"));
    interface->addButton(CreateTriggerParam("Reset Snakes", NULL)
                          .oscReceiver(mName,"snakereset"))->registerCallback( this, &Eclipse::resetSnakes );
    interface->addParam(CreateFloatParam("Snake Length", &mSnakeLength)
                        .minValue(1.0f)
                        .maxValue(mDim*4.0f)
                        .oscReceiver(mName,"snakelength"));
    interface->addParam(CreateFloatParam("Snake Min Speed", &mMinSnakeSpeed)
                        .minValue(mDim/2.0f)
                        .maxValue(mDim*2.0f)
                        .oscReceiver(mName,"snakeminspeed"));
    interface->addParam(CreateFloatParam("Snake Max Speed", &mMaxSnakeSpeed)
                        .minValue(mDim/2.0f)
                        .maxValue(mDim*2.0f)
                        .oscReceiver(mName,"snakemaxspeed"));
    interface->addParam(CreateBoolParam("Snake Random Speed", &mSnakeRandomSpeed)
                        .oscReceiver(mName,"snakespeedrandom"));
    interface->addParam(CreateFloatParam("Snake Speed Multiplier", &mSnakeSpeedMultiplier)
                        .minValue(0.0f)
                        .maxValue(100.0f)
                        .oscReceiver(mName,"snakespeedmutli"));
    interface->addEnum(CreateEnumParam("Snake Direction", (int*)&mSnakeDirMode)
                       .maxValue(SNAKEDIR_COUNT)
                       .oscReceiver(mName,"snakedirmode")
                       .isVertical());
    interface->addParam(CreateBoolParam("Snakes Rounded", &mSnakeRounded)
                        .oscReceiver(mName,"snakerounded"));
    interface->addParam(CreateColorParam("Color", &mColor, kMinColor, kMaxColor)
                        .oscReceiver(mName,"color"));
}

void Eclipse::setupDebugInterface()
{
}

void Eclipse::reset()
{
    resetSnakes();
}

Vec2f Eclipse::getPoint()
{
    return Vec2f::zero();
}

void Eclipse::resize()
{
}

void Eclipse::update(double dt)
{
    /*
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::color( ColorAf::white() );
    glLineWidth(1.0f);
    
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    
    // Get data
    
    const float * timeData = audioInput.getFft()->getData();
    const float * amplitude = audioInput.getFft()->getAmplitude();
    const int32_t	binSize = audioInput.getFft()->getBinSize();
    const float *	imaginary = audioInput.getFft()->getImaginary();
    const float *	phase = audioInput.getFft()->getPhase();
    const float *	real = audioInput.getFft()->getReal();
    const int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    assert( binSize < NUM_POINTS );
    
    // Get dimensions
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    */
    
    if( mMode == MODE_SNAKES )
    {
        updateSnakes(dt);
    }
}

void Eclipse::draw()
{
    /*
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    const float* amplitude = audioInput.getFft()->getAmplitude();
    const int32_t binSize = audioInput.getFft()->getBinSize();
    */
    
    gl::enable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    gl::enable( GL_POLYGON_SMOOTH );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    gl::color( ColorAf::white() );
    glLineWidth(4.0f);
    
    /*
    const float windowHeight = mParentScene->getApp()->getViewportHeight();
    const float windowWidth = mParentScene->getApp()->getViewportWidth();
    
    const float centerX = windowWidth/2.0f;
    const float centerY = windowHeight/2.0f;
    */
    //gl::enableAdditiveBlending();
    gl::disableDepthWrite();
    gl::disableDepthRead();
    
    if( mShowTestBackground )
        gl::draw( mTestBackground );
    
    switch( mMode )
    {
        case MODE_LINES:
            drawLines();
            break;
            
        case MODE_RECTS:
            drawRects();
            break;
        
        case MODE_CUBES:
            drawCubes();
            break;
            
        case MODE_SNAKES:
            drawSnakes();
            break;
            
        default:
            break;
    }
}

void Eclipse::drawLines()
{
    int x = mXOffset;
    int y = mYOffset;
    
    //glBegin(GL_LINES);
    // row 1
    for( int i=0; i < ROW1_RECTS; ++i )
    {
        drawRowLines(x+mDim*i, y, i);
    }
    // row 2
    x += mDim;
    y += mDim;
    for( int i=0; i < ROW2_RECTS; ++i )
    {
        drawRowLines(x+mDim*i, y, i + ROW1_RECTS);
        //gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
    // row 3
    x += mDim;
    y += mDim;
    for( int i=0; i < ROW3_RECTS; ++i )
    {
        drawRowLines(x+mDim*i, y, i + ROW1_RECTS + ROW2_RECTS);
        //gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
    //glEnd();
}

void Eclipse::drawRects()
{
    int x = mXOffset;
    int y = mYOffset;
    
    float thickness = 0.0f;
    
    // row 1
    for( int i=0; i < ROW1_RECTS; ++i )
    {
        if(mRandomizeFaces || i % 2 == 0)
        {
            thickness = getThicknessValue(i);
        }
        drawRowRects(x+mDim*i, y, thickness);
    }
    // row 2
    x += mDim;
    y += mDim;
    for( int i=0; i < ROW2_RECTS; ++i )
    {
        if(mRandomizeFaces || i % 2 == 0)
        {
            thickness = getThicknessValue( i + ROW1_RECTS );
        }
        drawRowRects(x+mDim*i, y, thickness);

        //gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
    // row 3
    x += mDim;
    y += mDim;
    for( int i=0; i < ROW3_RECTS; ++i )
    {
        if(mRandomizeFaces || i % 2 == 0)
        {
            thickness = getThicknessValue( i + ROW1_RECTS + ROW2_RECTS );
        }
        drawRowRects(x+mDim*i, y, thickness);

        //gl::drawStrokedRect(Rectf(x+mDim*i,y,x+mDim*(i+1),y+mDim));
    }
}

float Eclipse::getThicknessValue(const int baseIndex)
{
    AudioInput& audioInput = mParentScene->getApp()->getAudioInput();
    const float* amplitude = audioInput.getFft()->getAmplitude();
    const int32_t binSize = audioInput.getFft()->getBinSize();
    const int32_t dataSize = audioInput.getFft()->getBinSize();
    const AudioInput::FftLogPlot& fftLogData = audioInput.getFftLogData();
    
    const int maxIndex = mUseFftData ? dataSize : binSize;
    
    const float maxThickness = ( mMode == MODE_LINES ) ? 5.0f : mMaxThickness;
    
    float thickness = 0.0f;
    int index = mRandomizeSignal ? Rand::randInt(maxIndex) : baseIndex;
    if( mUseFftData )
    {
        index = math<int>::max(2,index);
        thickness = fftLogData[index].y * maxThickness * (1.0f + index * mSignalScale);
    }
    else
    {
        thickness = amplitude[index] * maxThickness * (1.0f + index * mSignalScale);
    }
    
    return thickness;
}

void Eclipse::drawRowRects(const float x1, const float y1, const float thickness)
{
    const float x2 = x1+mDim;
    const float y2 = y1+mDim;
    
    // outer
    if( mInverse )
    {
        gl::color( ColorA(0.0f,0.0f,0.0f,1.0f));
    }
    else
    {
        gl::color( ColorA(1.0f,1.0f,1.0f,1.0f));
    }
    gl::drawSolidRect(Rectf(x1,y1,x2,y2));
    
    // inner
    if( !mInverse )
    {
        gl::color( ColorA(0.0f,0.0f,0.0f,1.0f));
    }
    else
    {
        gl::color( ColorA(1.0f,1.0f,1.0f,1.0f));
    }
    gl::drawSolidRect(Rectf(x1+thickness,y1+thickness,x2-thickness,y2-thickness));
    
    if( mDrawInnerLines )
    {
        gl::drawLine(Vec2f(x1,y1), Vec2f(x1+thickness,y1+thickness));
        gl::drawLine(Vec2f(x2,y1), Vec2f(x2-thickness,y1+thickness));
        gl::drawLine(Vec2f(x1,y2), Vec2f(x1+thickness,y2-thickness));
        gl::drawLine(Vec2f(x2,y2), Vec2f(x2-thickness,y2-thickness));
    }
}

void Eclipse::drawRowLines(const float x1, const float y1, const int rectIndex)
{
    const float x2 = x1+mDim;
    const float y2 = y1+mDim;
    
    gl::color( ColorA(1.0f,1.0f,1.0f,1.0f));
    
    float thickness = getThicknessValue( rectIndex );
    //gl::drawLine(Vec2f(x1,y1), Vec2f(x1+thickness,y1+thickness));
    gl::drawSolidRect(Rectf(x1,y1,x1+thickness,y2)); // vleft
    thickness = getThicknessValue( rectIndex+1 );
    gl::drawSolidRect(Rectf(x1,y1,x2,y1+thickness)); // htop
    thickness = getThicknessValue( rectIndex+2 );
    gl::drawSolidRect(Rectf(x2-thickness,y1,x2,y2)); // vright
    thickness = getThicknessValue( rectIndex+3 );
    gl::drawSolidRect(Rectf(x1,y2-thickness,x2,y2)); // hbot
}

void Eclipse::drawCubes()
{
    gl::pushMatrices();
    
    int x = mXOffset;
    int y = mYOffset;
    const float halfDim = mDim/2.0f;
    const Vec3f dimVec = Vec3f(mDim,mDim,mDim);
    
    gl::translate(x+halfDim,y+halfDim,-halfDim);
    // row 1
    gl::pushMatrices();
    for( int i=0; i < ROW1_RECTS; ++i )
    {
        gl::pushMatrices();
        gl::rotate(Vec3f(0.0f,getElapsedSeconds()*10.0f,0.0f));
        gl::drawStrokedCube(Vec3f(0.0f,0.0f,0.0f), dimVec);
        gl::popMatrices();
        gl::translate(mDim, 0.0f, 0.0f);
    }
    gl::popMatrices();
    
    // row 2
    gl::translate(mDim,mDim,0.0f);
    gl::pushMatrices();
    for( int i=0; i < ROW2_RECTS; ++i )
    {
        gl::pushMatrices();
        gl::rotate(Vec3f(0.0f,getElapsedSeconds()*-1.0f,0.0f));
        gl::drawStrokedCube(Vec3f(0.0f,0.0f,0.0f), dimVec);
        gl::popMatrices();
        gl::translate(mDim, 0.0f, 0.0f);
    }
    gl::popMatrices();
    
    // row 3
    gl::translate(mDim,mDim,0.0f);
    gl::pushMatrices();
    for( int i=0; i < ROW3_RECTS; ++i )
    {
        gl::pushMatrices();
        gl::rotate(Vec3f(0.0f,getElapsedSeconds()*50.0f,0.0f));
        gl::drawStrokedCube(Vec3f(0.0f,0.0f,0.0f), dimVec);
        gl::popMatrices();
        gl::translate(mDim, 0.0f, 0.0f);
    }
    gl::popMatrices();
    
    gl::popMatrices();
}

#pragma mark - Snakes

bool Eclipse::resetSnakes()
{
    // vertical
    for( int i=0; i < NUM_VLINES; ++i )
    {
        mVSnakes[i].mHead.x = mXOffset + mDim*i;
        
//        if( mSnakeRandomStart )
//        {
//            mVSnakes[i].mHead.y = Rand::randFloat( mParentScene->getApp()->getViewportHeight() );
//        }
//        else
        {
            mVSnakes[i].mHead.y = mParentScene->getApp()->getViewportHeight();
        }
        
        mVSnakes[i].mDir.x = 0.0f;
        
        switch( mSnakeDirMode )
        {
            case SNAKEDIR_UNISON:
                mVSnakes[i].mDir.y = -1.0f; // up
                break;
            case SNAKEDIR_ALTERNATE:
                mVSnakes[i].mDir.y = ( i % 2 == 0 ) ? 1.0f : -1.0f;
                break;
            case SNAKEDIR_RANDOM:
                mVSnakes[i].mDir.y = Rand::randBool() ? 1.0f : -1.0f;
                break;
                
            default:
                break;
        }
        
        mVSnakes[i].mSpeed = mSnakeRandomSpeed ? Rand::randFloat( mMinSnakeSpeed, mMaxSnakeSpeed ) : (mMinSnakeSpeed+mMaxSnakeSpeed)/2.0f;
        
        //mVSnakes[i].mLength = mSnakeLength;
    }
    
    // horizontal
    for( int i=0; i < NUM_HLINES; ++i )
    {
        mHSnakes[i].mHead.x = 0.0f;
        mHSnakes[i].mHead.y = mYOffset + mDim*i;
        
        mHSnakes[i].mDir.y = 0.0f;
        
        switch( mSnakeDirMode )
        {
            case SNAKEDIR_UNISON:
                mHSnakes[i].mDir.x = 1.0f; // right
                break;
            case SNAKEDIR_ALTERNATE:
                mHSnakes[i].mDir.x = ( i % 2 == 0 ) ? 1.0f : -1.0f;
                break;
            case SNAKEDIR_RANDOM:
                mHSnakes[i].mDir.x = Rand::randBool() ? 1.0f : -1.0f;
                break;
                
            default:
                break;
        }
        
        mHSnakes[i].mSpeed = mSnakeRandomSpeed ? Rand::randFloat( mMinSnakeSpeed, mMaxSnakeSpeed ) : (mMinSnakeSpeed+mMaxSnakeSpeed)/2.0f;
        
        //mHSnakes[i].mLength = mSnakeLength;
    }
    /*
    mSnakes.clear();
    for( int i=0; i < mNumSnakes; ++i )
    {
        tSnake snake;
        const int row = Rand::randInt(NUM_HLINES);
        const int col = Rand::randInt(mMinColForRow[row],mMaxColForRow[row]);
        
        const int x = mXOffset + mDim*col;
        const int y = mYOffset + mDim*row;
        
        snake.mLength = mSnakeLength;
        
        //snake.mPath.moveTo( x, y );
        snake.mHead.x = x;
        snake.mHead.y = y;
        
        snake.mVel.x = 0.0f;
        snake.mVel.y = 0.0f;
        
        // random initial velocity
        // x or y
        if( Rand::randBool() )
        {
            // positive or negative
            snake.mVel.x = Rand::randBool() ? 1.0f : -1.0f;
            if( (snake.mVel.x > 0.0f && x == mXMax[row]) ||
               (snake.mVel.x < 0.0f && x == mXMin[row]) )
            {
                snake.mVel.x *= -1.0f;
            }
        }
        else
        {
            snake.mVel.y = Rand::randBool() ? 1.0f : -1.0f;
            if( (snake.mVel.y > 0.0f && y == mYMax[col]) ||
               (snake.mVel.y < 0.0f && y == mYMin[col]) )
            {
                snake.mVel.y *= -1.0f;
            }
        }
        
        snake.mBounds.x1 = mXMin[row];
        snake.mBounds.x2 = mXMax[row];
        snake.mBounds.y1 = mYMin[row];
        snake.mBounds.y2 = mYMax[row];
        
        console() << "snake " << i << ": " << x << "," << y;
        
        //for( int j=0; j < mNumSnakePoints; ++j )
        {
            //const Vec2f next = Vec2f(x,y) + snake.mVel * (snake.mLength/mNumSnakePoints);
            //snake.mPath.lineTo( next );
            const Vec2f next = Vec2f(x,y) + snake.mVel * (snake.mLength);
            snake.mTail = next;
            console() << " to " << next.x << "," << next.y;
        }
        console() << std::endl;
        
        snake.mVel *= Rand::randFloat( mMinSnakeSpeed, mMaxSnakeSpeed );
        
        mSnakes.push_back(snake);
    }
     */
    
    return false;
}

void Eclipse::updateSnakes(double dt)
{
    const float maxHeight = mYOffset + 3*mDim;
    const float maxWidth = mXOffset + 6*mDim;
    for( int i=0; i < NUM_VLINES; ++i )
    {
        tSnake& snake = mVSnakes[i];
        snake.mHead.y += snake.mDir.y * snake.mSpeed * mSnakeSpeedMultiplier * dt;
        
        if( (snake.mHead.y < (mYOffset-mSnakeLength)) && snake.mDir.y < 0.0f )
        {
            snake.mHead.y = maxHeight;
        }
        else if( (snake.mHead.y > (maxHeight+mSnakeLength)) && snake.mDir.y > 0.0f )
        {
            snake.mHead.y = mYOffset;
        }
    }
    
    for( int i=0; i < NUM_HLINES; ++i )
    {
        tSnake& snake = mHSnakes[i];
        snake.mHead.x += snake.mDir.x * snake.mSpeed * mSnakeSpeedMultiplier * dt;
        
        if( (snake.mHead.x < mXOffset-mSnakeLength) && snake.mDir.x < 0.0f )
        {
            snake.mHead.x = maxWidth;
        }
        else if( (snake.mHead.x > maxWidth+mSnakeLength) && snake.mDir.x > 0.0f )
        {
            snake.mHead.x = mXOffset;
        }
    }
    /*
    for( int i=0; i < mNumSnakes; ++i )
    {
        tSnake& snake = mSnakes[i];
        snake.mHead.x += snake.mVel.x * dt;
        snake.mHead.y += snake.mVel.y * dt;
        
        snake.mHead.x = math<int>::max( snake.mBounds.x1, snake.mHead.x );
        snake.mHead.x = math<int>::min( snake.mBounds.x2, snake.mHead.x );
        snake.mHead.y = math<int>::max( snake.mBounds.y1, snake.mHead.y );
        snake.mHead.y = math<int>::min( snake.mBounds.y2, snake.mHead.y );
        
        /*
        snake.mTail.x += snake.mVel.x * dt;
        snake.mTail.y += snake.mVel.y * dt;
        
        snake.mTail.x = math<int>::max( snake.mBounds.x1, snake.mTail.x );
        snake.mTail.x = math<int>::min( snake.mBounds.x2, snake.mTail.x );
        snake.mTail.y = math<int>::max( snake.mBounds.y1, snake.mTail.y );
        snake.mTail.y = math<int>::min( snake.mBounds.y2, snake.mTail.y );
        *
        
    }
    */
    /*
     for( int i=0; i < mNumSnakes; ++i )
     {
     tSnake& snake = mSnakes[i];
     for( int j=0; j < mNumSnakePoints+1; ++j )
     {
     const Vec2f prev = snake.mPath.getPoint(j);
     const Vec2f next = prev + snake.mVel * dt;
     snake.mPath.setPoint(j, next);
     }
     }
     */
}

void Eclipse::drawSnakes()
{
    gl::color( mColor );
    
    for( int i=0; i < NUM_VLINES; ++i )
    {
        tSnake& snake = mVSnakes[i];
        
        Vec2f tail( snake.mHead - mSnakeLength*snake.mDir );
        //gl::drawLine(snake.mHead, tail);
        float thickness = getThicknessValue( i );
        Rectf rect = ( snake.mDir.y > 0.0f ) ? Rectf(tail,snake.mHead) : Rectf(snake.mHead,tail);
        rect.inflate( Vec2f( thickness, 0.0f ) );
        if( mSnakeRounded )
        {
            gl::drawSolidRoundedRect(rect, thickness/2.0f);
        }
        else
        {
            gl::drawSolidRect(rect);
        }
    }
    for( int i=0; i < NUM_HLINES; ++i )
    {
        tSnake& snake = mHSnakes[i];
        
        Vec2f tail( snake.mHead - mSnakeLength*snake.mDir );
        float thickness = getThicknessValue( i );
        Rectf rect = ( snake.mDir.x > 0.0f ) ? Rectf(tail,snake.mHead) : Rectf(snake.mHead,tail);
        rect.inflate( Vec2f( 0.0f, thickness ) );
        if( mSnakeRounded )
        {
            gl::drawSolidRoundedRect(rect, thickness/2.0f);
        }
        else
        {
            gl::drawSolidRect(rect);
        }
    }
    /*
    for( int i=0; i < mNumSnakes; ++i )
    {
        //gl::draw( mSnakes[i].mPath );
        float directionX = mSnakes[i].mVel.x > 0.0f ? 1.0f : mSnakes[i].mVel.x < 0.0f ? -1.0f : 0.0f;
        float directionY = mSnakes[i].mVel.y > 0.0f ? 1.0f : mSnakes[i].mVel.y < 0.0f ? -1.0f : 0.0f;
        Vec2f tail( mSnakes[i].mHead.x + mSnakes[i].mLength*directionX, mSnakes[i].mHead.y + mSnakes[i].mLength*directionY );
        gl::drawLine(mSnakes[i].mHead, tail);
    }
     */
}
