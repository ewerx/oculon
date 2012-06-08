//
//  SolFrame.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-05-24.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "SolFrame.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"
#include "TextureManager.h"

using namespace boost::posix_time;
using namespace ci;
using namespace ci::app;

using std::istringstream;
using std::stringstream;
using std::string;
using std::locale;


SolFrame::SolFrame()
: mTimeStampFirst(pos_infin)
, mTimeStampLast(neg_infin)
{
    for (int i=0; i < SOURCE_COUNT; ++i)
    {
        mTextures[i] = gl::Texture();
    }
}

bool SolFrame::init( const fs::path& filePath )
{
    string filename = filePath.filename().string();
    // e.g.
    // date...._time.._res._src.
    // 20120518_234611_1024_0131
    string datetime = filename.substr(0,15);
    string srcname = filename.substr(20,4); // 21 for 1024
    
    // format: 2010518_234611
    time_input_facet* p_facet = new time_input_facet("%Y%m%d_%H%M%S");
    // locale takes ownership of p_facet
    locale timeStampLocale(locale(""), p_facet);
    stringstream timestampLine( datetime );
    timestampLine.imbue(timeStampLocale);
    ptime timeStamp;
    timestampLine >> timeStamp;
    
    bool imageAdded = false;
    
    eImageSource src = getSourceByName(srcname);
    if( src < SOURCE_COUNT )
    {
        if( mTexturePaths[src].empty() )
        {
            console() << filePath.string() << std::endl;
            mTexturePaths[src] = filePath;
            // async load
            mTextures[src] = ph::TextureManager::getInstance().fetch( filePath.string(), ci::gl::Texture::Format(), true );
            if( timeStamp < mTimeStampFirst )
            {
                mTimeStampFirst = timeStamp;
            }
            else if( timeStamp > mTimeStampLast )
            {
                mTimeStampLast = timeStamp;
            }
            imageAdded = true;
        }
        else
        {
            // if we already have an image from this source, this image is for the next frame
            // don't add it

            // sanity check for missing images in this frame
            for (int i=0; i < SOURCE_COUNT; ++i)
            {
                if( mTexturePaths[i].empty() )
                {
                    console() << "[sol] WARNING: frame missing image source " << i << std::endl;
                }
            }
        }
    }
    else
    {
        console() << "[sol] WARNING: unknown source type from string: " << srcname << std::endl;
        imageAdded = true; // pretend we added it to continue on this frame
    }
    
    return imageAdded;
}

bool SolFrame::loadTextures()
{
    for( int i=0; i < SOURCE_COUNT; ++i )
    {
        if( !mTexturePaths[i].empty() )
        {
            mTextures[i] = ph::TextureManager::getInstance().fetch( mTexturePaths[i].string(), ci::gl::Texture::Format(), true );
        }
    }
    
    return true;
}

void SolFrame::unloadTextures()
{
    for( int i=0; i < SOURCE_COUNT; ++i )
    {
        //console() << "[sol] clearing " << mTexturePaths[i].string() << " state: ";
        if( ph::TextureManager::getInstance().isLoading( mTexturePaths[i].string() ) )
            console() << "[sol] WARNING: texture is still queued for loading" << std::endl;//"LOADING";
        //else if( ph::TextureManager::getInstance().isLoaded( mTexturePaths[i].string() ) )
        //    console() << "LOADED";
        //else 
        //    console() << "NONE";
        //console() << std::endl;
        
        mTextures[i].reset();
        ph::TextureManager::getInstance().removeTexture( mTexturePaths[i].string() );
    }
}

int SolFrame::getImageCount() const
{
    int count = 0;
    for (int i=0; i < SOURCE_COUNT; ++i)
    {
        if( mTextures[i] != gl::Texture() )
        {
            ++count;
        }
    }
    
    return count;
}

SolFrame::eImageSource SolFrame::getSourceByName( const std::string& name )
{
    const uint32_t srcNumeric = fromString<uint32_t>(name);
    switch( srcNumeric )
    {
        //case 94:
        //    return SOURCE_0094;
        case 131:
            return SOURCE_0131;
        case 171:
            return SOURCE_0171;
        case 193:
            return SOURCE_0193;
        //case 211:
        //    return SOURCE_0211;
        case 304:
            return SOURCE_0304;
        case 335:
            return SOURCE_0335;
        //case 1600:
        //    return SOURCE_1600;
        case 1700:
            return SOURCE_1700;
        default:
            return SOURCE_COUNT;
    }
}

ci::gl::Texture SolFrame::getTexture( const SolFrame::eImageSource src )
{
    if( src < SOURCE_COUNT )
    {
        if( mTextures[src] == gl::Texture() && !mTexturePaths[src].empty() )
        {
            mTextures[src] = ph::TextureManager::getInstance().fetch( mTexturePaths[src].string(), ci::gl::Texture::Format(), true );
        }
        return mTextures[src];
    }
    
    return gl::Texture();
}
