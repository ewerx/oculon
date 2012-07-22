#pragma once
#include "cinder/Color.h"
#include "cinder/Vector.h"

static const float kTwoPi           = 2.0f*M_PI;
static const float kWindowWidth		= 640.0f;
static const float kWindowHeight	= 480.0f;

static const float kCaptureWidth	= 640.0f;
static const float kCaptureHeight	= 480.0f;

static const ci::Color kClearColor	= ci::Color(0.0f, 0.0f, 0.0f);
static const ci::ColorA kMinColor   = ci::ColorA(0.0f,0.0f,0.0f,0.0f);
static const ci::ColorA kMaxColor   = ci::ColorA(1.0f,1.0f,1.0f,1.0f);

static const float kCaptureFramerate = 25.0f;

static const int OSC_ADDRESS_SIZE   = 256;

enum eLogLevel
{
    LOGLEVEL_SILENT,
    LOGLEVEL_QUIET,
    LOGLEVEL_VERBOSE,
};

struct tMetropolisPattern
{
    // grid
    const int numSegs = 14;
    float segWidth;// = windowWidth / numSegs;
    
    // fixed points
    float centerSidePointX;// = -0.8f*segWidth;
    float topBotSidePointX;// = 0.2f*segWidth;
    float topBotSidePointY;// = -(windowHeight/6.2f);
    ci::Vec2f p0;//(centerX, centerY); // center
    ci::Vec2f p1;//(centerSidePointX, centerY); // left side
    ci::Vec2f p2;//(windowWidth - centerSidePointX, centerY); // right side
    ci::Vec2f p3;//(topBotSidePointX, topBotSidePointY); // top left
    ci::Vec2f p4;//(topBotSidePointX, windowHeight - topBotSidePointY); // bottom left
    ci::Vec2f p5;//(windowWidth - topBotSidePointX, topBotSidePointY); // top right
    ci::Vec2f p6;//(windowWidth - topBotSidePointX, windowHeight - topBotSidePointY); // bottom right

    void init( const float windowWidth, const float windowHeight )
    {
        const float centerX = windowWidth/2.0f;
        const float centerY = windowHeight/2.0f;
        
        segWidth = windowWidth / numSegs;
                // fixed points
        centerSidePointX = -0.8f*segWidth;
        topBotSidePointX = 0.2f*segWidth;
        topBotSidePointY = -(windowHeight/6.2f);
        p0 = ci::Vec2f(centerX, centerY); // center
        p1 = ci::Vec2f(centerSidePointX, centerY); // left side
        p2 = ci::Vec2f(windowWidth - centerSidePointX, centerY); // right side
        
        p3 = ci::Vec2f(topBotSidePointX, topBotSidePointY); // top left
        p4 = ci::Vec2f(topBotSidePointX, windowHeight - topBotSidePointY); // bottom left
        p5 = ci::Vec2f(windowWidth - topBotSidePointX, topBotSidePointY); // top right
        p6 = ci::Vec2f(windowWidth - topBotSidePointX, windowHeight - topBotSidePointY); // bottom right
        
        
    }
};

#define DEV_MODE
