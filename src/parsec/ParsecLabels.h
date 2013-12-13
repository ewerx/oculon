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
              std::string spectrum,
              const ci::Font &font );
        void update( const ci::Camera &cam, float scale, float screenWidth, float screenHeight );
        void draw( float alpha );
        
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
    void update(const ci::Camera &cam, float screenWidth, float screenHeight);
    //void updateAudioResponse(
    void draw(float screenWidth, float screenHeight, float alpha);

    //TODO: refactor
    void addLabel( Label* label );
    
private:
    std::vector<Label*> mLabels;
    
};