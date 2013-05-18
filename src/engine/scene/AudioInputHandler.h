//
//  AudioInputHandler.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2013-05-17.
//
//

#ifndef __Oculon__AudioInputHandler__
#define __Oculon__AudioInputHandler__

#include "AudioInput.h"

#include "cinder/gl/Fbo.h"
#include "cinder/Timeline.h"

#include <vector>


class AudioInputHandler
{
public:
    AudioInputHandler();
    virtual ~AudioInputHandler();
    
    void setup();
//    void setupInterface();
    void update(double dt, AudioInput& audioInput);
    
    void drawDebug(const Vec2f& size);
    
    ci::gl::Texture getTexture() { return mAudioFbo.getTexture(); }
    ci::gl::Fbo& getFbo() { return mAudioFbo; }
    
private:
    
private:
    // Texture
    int                 mAudioFboDim;
    ci::Vec2f           mAudioFboSize;
    ci::Area            mAudioFboBounds;
    ci::gl::Fbo         mAudioFbo;
    
    // FALLOFF
#define AUDIO_FALLOFF_MODE_TUPLE \
AUDIO_FALLOFF_MODE_ENTRY( "None", FALLOFF_NONE ) \
AUDIO_FALLOFF_MODE_ENTRY( "Linear", FALLOFF_LINEAR ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Quad", FALLOFF_OUTQUAD ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Expo", FALLOFF_OUTEXPO ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Back", FALLOFF_OUTBACK ) \
AUDIO_FALLOFF_MODE_ENTRY( "Out-Bounce", FALLOFF_OUTBOUNCE ) \
AUDIO_FALLOFF_MODE_ENTRY( "OutIn-Expo", FALLOFF_OUTINEXPO ) \
AUDIO_FALLOFF_MODE_ENTRY( "OutIn-Back", FALLOFF_OUTINBACK ) \
//end tuple
    
    enum eFalloffMode
    {
#define AUDIO_FALLOFF_MODE_ENTRY( nam, enm ) \
enm,
        AUDIO_FALLOFF_MODE_TUPLE
#undef  AUDIO_FALLOFF_MODE_ENTRY
        
        FALLOFFMODE_COUNT
    };
    eFalloffMode        mFalloffMode;
    
    typedef std::function<float (float)> tEaseFn;
    tEaseFn getFalloffFunction();
    tEaseFn getReverseFalloffFunction();

    std::vector< ci::Anim<float> >    mFftFalloff;
    float               mFalloffTime;
    
    
};

#endif /* defined(__Oculon__AudioInputHandler__) */
