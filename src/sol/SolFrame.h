//
//  SolFrame.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-05-24.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "cinder/Filesystem.h"
#include "cinder/gl/Texture.h"
#include <vector>
#include <boost/date_time.hpp>

class SolFrame
{
public:
    enum eImageSource
    {
        //SOURCE_0094,
        SOURCE_0131,
        SOURCE_0171,
        SOURCE_0193,
        //SOURCE_0211,
        SOURCE_0304,
        SOURCE_0335,
        //SOURCE_1600,
        SOURCE_1700,
        
        SOURCE_COUNT
    };
public:
    SolFrame();
    ~SolFrame();
    
    bool init( const ci::fs::path& filename );
    bool loadTextures();
    void unloadTextures();
    int getImageCount() const;
    ci::gl::Texture getTexture( const eImageSource src );
    
private:
    eImageSource getSourceByName( const std::string& name );
private:
    
    
    ci::gl::Texture mTextures[SOURCE_COUNT];
    ci::fs::path mTexturePaths[SOURCE_COUNT];
    boost::posix_time::ptime mTimeStampFirst;
    boost::posix_time::ptime mTimeStampLast;
};

