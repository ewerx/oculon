/*
 *  Barcode.h
 *  Oculon
 *
 *  Created by Ehsan on 11-10-22.
 *  Copyright 2011 ewerx. All rights reserved.
 *
 */

#pragma once

#include "SubScene.h"
#include "cinder/Timeline.h"
#include "cinder/PolyLine.h"
#include "cinder/Color.h"

class Scene;

class Barcode : public SubScene
{
public:
    Barcode(Scene* scene);
    virtual ~Barcode();

    void setup();
    void setupInterface();
    void setupDebugInterface();
    void resize();
    void reset();
    void update(double dt);
    void draw();
    
private:
    void drawBar(const int index);
    
    float getPositionOffset( const int barIndex, const int lineIndex, const float fftx, const float ffty, const float fftDataSize, const float barRange );
    
    ci::ColorAf mColor;
    float       mBaseWidth;
    float       mMaxWidth;
    float       mThreshold;
    
    bool        mWidthByFft;
    bool        mAlphaByFft;
    
    int         mNumBars;
    int         mBarGap;
    bool        mVertical;
    
    struct tLine
    {
        ci::Anim<float> mWidth;
        ci::Anim<float> mAlpha;
    };
    enum
    {
        MAX_BARS = 64,
        MAX_LINES = 256
    };
    tLine mLines[MAX_BARS][MAX_LINES];
    
    // POSITION MODE
#define BARCODE_POS_MODE_TUPLE \
BARCODE_POS_MODE_ENTRY( "Fixed-Center", POSITION_FIXED_CENTER ) \
BARCODE_POS_MODE_ENTRY( "Fixed-Rand", POSITION_FIXED_RANDOM ) \
BARCODE_POS_MODE_ENTRY( "Shift-Rand", POSITION_SHIFT_RANDOM ) \
BARCODE_POS_MODE_ENTRY( "Shift-Linear", POSITION_SHIFT_LINEAR ) \
//end tuple
    enum ePositionMode
    {
#define BARCODE_POS_MODE_ENTRY( nam, enm ) \
enm,
        BARCODE_POS_MODE_TUPLE
#undef  BARCODE_POS_MODE_ENTRY
        
        POSITION_COUNT
    };
    ePositionMode mPositionMode;
    
    // WIDTH MODE
#define BARCODE_WIDTH_MODE_TUPLE \
BARCODE_WIDTH_MODE_ENTRY( "Fixed", WIDTH_FIXED ) \
BARCODE_WIDTH_MODE_ENTRY( "FixedByFreq", WIDTH_FIXED_FREQ ) \
BARCODE_WIDTH_MODE_ENTRY( "VariableByFreq", WIDTH_AUDIO_FREQ ) \
//end tuple
    enum eWidthMode
    {
#define BARCODE_WIDTH_MODE_ENTRY( nam, enm ) \
enm,
        BARCODE_WIDTH_MODE_TUPLE
#undef  BARCODE_WIDTH_MODE_ENTRY
        
        WIDTH_MODE_COUNT
    };
    eWidthMode mWidthMode;
};
