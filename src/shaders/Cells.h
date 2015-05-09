//
//  Cells.h
//  Oculon
//
//  Created by Ehsan on 13-11-15.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "Scene.h"
#include "TextureShaders.h"
#include "FragShader.h"

//
// Cellular Noise
//
class Cells : public TextureShaders
{
public:
    Cells();
    virtual ~Cells();
    
private:
    void setupShaders() override;
    
private:
    
    class MultiLayer: public FragShader
    {
    public:
        MultiLayer();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        float               mAudioResponseFreqMin;
        float               mAudioResponseFreqMax;
        
#define AUDIO_RESPONSE_TYPE_TUPLE \
AUDIO_RESPONSE_TYPE_ENTRY( "None", AUDIO_RESPONSE_NONE ) \
AUDIO_RESPONSE_TYPE_ENTRY( "SingleBand", AUDIO_RESPONSE_SINGLE ) \
AUDIO_RESPONSE_TYPE_ENTRY( "MultiBand", AUDIO_RESPONSE_MULTI ) \
//end tuple
        
        enum eAudioRessponseType
        {
#define AUDIO_RESPONSE_TYPE_ENTRY( nam, enm ) \
enm,
            AUDIO_RESPONSE_TYPE_TUPLE
#undef  AUDIO_RESPONSE_TYPE_ENTRY
            
            AUDIO_RESPONSE_TYPE_COUNT
        };
        eAudioRessponseType mAudioResponseType;
        
        float mZoom;
        float mHighlight;
        
        enum { CELLS_NUM_LAYERS = 7 };
        
        float mTimeStep[CELLS_NUM_LAYERS];
        float mFrequency[CELLS_NUM_LAYERS];
        float mTime[CELLS_NUM_LAYERS];
        
        float mIntensity;
    };

    class Bubbles: public FragShader
    {
    public:
        Bubbles();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
    };
    
    class GravityFieldShader : public FragShader
    {
    public:
        GravityFieldShader();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
        int mPoints;
        int mMode;
        
    };
    
};
