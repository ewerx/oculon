//
//  ParsecLabels.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12/12/2013.
//
//

#pragma once

#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/Utilities.h"

#include "cinder/Camera.h"
#include "cinder/Sphere.h"

#include "Scene.h"
#include "AudioInputHandler.h"

class ParsecLabels
{
public:
    class Label
    {
    public:
        Label();
        
        Label( ci::Vec3f pos,
              float absMag,
              std::string name,
              std::string dataLine,
              const ci::Font &font );
        void update( const ci::Camera &cam, float scale, float screenWidth, float screenHeight );
        void draw( float alpha, bool audioResponsive );
        
        ci::Vec3f	mInitPos;
        ci::Vec3f	mPos;
        ci::Sphere	mSphere;
        
        ci::Vec2f	mScreenPos;
        float		mScreenRadius;
        float		mDistToCam;
        float		mDistToCamPer;
        
        std::string		mName;
        ci::gl::Texture mNameTex;
        
        bool		mIsSelected;
        
        float       mAudioPer;
    };
    
public:
    ParsecLabels();
    virtual ~ParsecLabels();
    
    void setup();
    void updateAudio(AudioInputHandler &audioInputHandler, float gain);
    void update(const ci::Camera &cam, float distance, float screenWidth, float screenHeight);

    //void updateAudioResponse(
    void draw(float screenWidth, float screenHeight);

    //TODO: refactor
    void addLabel( Label* label );
    
public:
    float mAlpha;
    bool mFadeByDistance;
    bool mAudioResponsive;
    
private:
    std::vector<Label*> mLabels;
    float mAttenuation;
    
};