//
//  LissajousShader.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-05-25.
//
//

#pragma once

#include "FragShader.h"

class LissajousShader : public FragShader
{
public:
    LissajousShader();
    
    virtual void setupInterface( Interface* interface, const std::string& name );
    virtual void setCustomParams( AudioInputHandler& audioInputHandler );
    
private:
    float mScale;
    int mFrequencyX;
    int mFrequencyY;
    
    float mFrequencyXShift;
    float mFrequencyYShift;
    
    int mResponseBandX;
    int mResponseBandY;
    
    ci::ColorAf mColor;
};