//
//  EaseCurveSelector.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2/6/2014.
//
//

#pragma once

class Interface;

class EaseCurveSelector
{
public:
    EaseCurveSelector();
    EaseCurveSelector(float maxDuration);
    virtual ~EaseCurveSelector();
    
    void setupInterface(Interface *interface, const std::string& name);
    
    typedef std::function<float (float)> tEaseFn;
    tEaseFn getEaseFunction();
    tEaseFn getReverseEaseFunction();
    
    float mDuration;
    
private:
    
#define ANIMCURVE_TUPLE \
ANIMCURVE_ENTRY( "Linear", EASE_LINEAR ) \
ANIMCURVE_ENTRY( "Out-Quad", EASE_OUTQUAD ) \
ANIMCURVE_ENTRY( "Out-Expo", EASE_OUTEXPO ) \
ANIMCURVE_ENTRY( "Out-Back", EASE_OUTBACK ) \
ANIMCURVE_ENTRY( "Out-Bounce", EASE_OUTBOUNCE ) \
ANIMCURVE_ENTRY( "OutIn-Expo", EASE_OUTINEXPO ) \
ANIMCURVE_ENTRY( "OutIn-Back", EASE_OUTINBACK ) \
//end tuple
    
    enum eAnimCurve
    {
#define ANIMCURVE_ENTRY( nam, enm ) \
enm,
        ANIMCURVE_TUPLE
#undef  ANIMCURVE_ENTRY
        
        ANIMCURVE_COUNT
    };
    eAnimCurve mSelectedCurve;
    
    float mMaxDuration;
    bool mReverseEase;
};
