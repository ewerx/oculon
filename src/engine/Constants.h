#pragma once
#include "cinder/Color.h"

static const float kTwoPi           = 2.0f*M_PI;
static const float kWindowWidth		= 640.0f;
static const float kWindowHeight	= 480.0f;

static const float kCaptureWidth	= 640.0f;
static const float kCaptureHeight	= 480.0f;

static const ci::Color kClearColor	= ci::Color(0.0f, 0.0f, 0.0f);

static const float kCaptureFramerate = 25.0f;

static const int OSC_ADDRESS_SIZE   = 256;

enum eLogLevel
{
    LOGLEVEL_SILENT,
    LOGLEVEL_QUIET,
    LOGLEVEL_VERBOSE,
};

#define DEV_MODE
