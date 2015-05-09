//
//  CircleWave.h
//  Oculon
//
//  Created by Ehsan on 13-10-27.
//  Copyright 2013 ewerx. All rights reserved.
//

#pragma once

#include "cinder/Cinder.h"
#include "TextureShaders.h"
#include "FragShader.h"

//
// CircleWave
//
class CircleWave : public TextureShaders
{
public:
    CircleWave();
    virtual ~CircleWave();
    
private:
    void setupShaders() override;
    
private:

    #pragma mark -
    class Circle: public FragShader
    {
    public:
        Circle();

        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );

    private:
        float mSeparation;
        float mDetail;
        int mStrands;
        float mScale;
        bool mColorSeparation;
        float mThickness;

        // background
//#define CIRCLEWAVE_BG_REACTION_TUPLE \
// //CIRCLEWAVE_BG_REACTION_ENTRY( "None", BG_REACTION_NONE ) \
// //CIRCLEWAVE_BG_REACTION_ENTRY( "White", BG_REACTION_WHITE ) \
// //CIRCLEWAVE_BG_REACTION_ENTRY( "Color", BG_REACTION_COLOR ) \
// ////end tuple
//
//    enum eBackgroundReaction
//    {
//#define CIRCLEWAVE_BG_REACTION_ENTRY( nam, enm ) \
// //enm,
//        CIRCLEWAVE_BG_REACTION_TUPLE
//#undef  CIRCLEWAVE_BG_REACTION_ENTRY
//
//        BG_REACTION_COUNT
//    };
//    eBackgroundReaction mBackgroundReaction;

        bool mBackgroundFlash;
    };
    
    #pragma mark -
    class Spark: public FragShader
    {
    public:
        Spark();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
    };
    
    #pragma mark -
    class Trapezium: public FragShader
    {
    public:
        Trapezium();
        
        virtual void setupInterface( Interface* interface, const std::string& name );
        virtual void update(double dt);
        virtual void setCustomParams( AudioInputHandler& audioInputHandler );
        
    private:
    };
};

