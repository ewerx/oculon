//
//  EaseCurveSelector.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2/6/2014.
//
//

#include "EaseCurveSelector.h"
#include "Interface.h"
#include "cinder/Easing.h"
#include <vector>

using namespace ci;
using namespace std;

EaseCurveSelector::EaseCurveSelector()
: mSelectedCurve(EASE_LINEAR)
, mMaxDuration(10.0f)
, mDuration(0.0f)
, mReverseEase(false)
{
}

EaseCurveSelector::EaseCurveSelector(float maxDuration)
: mSelectedCurve(EASE_LINEAR)
, mMaxDuration(maxDuration)
, mDuration(0.0f)
, mReverseEase(false)
{
}

EaseCurveSelector::~EaseCurveSelector()
{
}

void EaseCurveSelector::setupInterface(Interface *interface, const std::string &name)
{
    interface->addParam(CreateFloatParam( "anim_time", &mDuration )
                        .minValue(0.0f)
                        .maxValue(mMaxDuration)
                        .oscReceiver(name));
    vector<string> animCurveNames;
#define ANIMCURVE_ENTRY( nam, enm ) \
animCurveNames.push_back(nam);
    ANIMCURVE_TUPLE
#undef  ANIMCURVE_ENTRY
    interface->addEnum(CreateEnumParam( "anim_curve", (int*)(&mSelectedCurve) )
                       .maxValue(ANIMCURVE_COUNT)
                       .isVertical()
                       .oscReceiver(name), animCurveNames);
    interface->addParam(CreateBoolParam( "anim_reverse", &mReverseEase)
                        .oscReceiver(name));
}

EaseCurveSelector::tEaseFn EaseCurveSelector::getEaseFunction()
{
    if (mReverseEase)
    {
        return getReverseEaseFunction();
    }
    
    switch( mSelectedCurve )
    {
        case EASE_LINEAR: return EaseNone();
        case EASE_OUTQUAD: return EaseOutQuad();
        case EASE_OUTEXPO: return EaseOutExpo();
        case EASE_OUTBACK: return EaseOutBack();
        case EASE_OUTBOUNCE: return EaseOutBounce();
        case EASE_OUTINEXPO: return EaseOutInExpo();
        case EASE_OUTINBACK: return EaseOutInBack();
            
        default: return EaseNone();
    }
}

EaseCurveSelector::tEaseFn EaseCurveSelector::getReverseEaseFunction()
{
    switch( mSelectedCurve )
    {
        case EASE_LINEAR: return EaseNone();
        case EASE_OUTQUAD: return EaseInQuad();
        case EASE_OUTEXPO: return EaseInExpo();
        case EASE_OUTBACK: return EaseInBack();
        case EASE_OUTBOUNCE: return EaseInBounce();
        case EASE_OUTINEXPO: return EaseInOutExpo();
        case EASE_OUTINBACK: return EaseInOutBack();
            
        default: return EaseNone();
    }
}
