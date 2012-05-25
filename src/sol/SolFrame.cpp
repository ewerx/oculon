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

using namespace boost::posix_time;
using namespace ci;
using namespace ci::app;

using std::istringstream;
using std::stringstream;
using std::string;
using std::locale;


SolFrame::SolFrame()
{
    for (int i=0; i < SOURCE_COUNT; ++i)
    {
        mTexture[i] = gl::Texture();
    }
}

bool SolFrame::addImage( const std::string& filename, ci::gl::Texture texture )
{
    // e.g.
    // date...._time.._res._src.
    // 20120518_234611_1024_0131
    string datetime = filename.substr(0,15);
    string srcname = filename.substr(21,4);
    
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
        if( mTexture[src] == gl::Texture() )
        {
            mTexture[src] = texture;
            if( timeStamp < mTimeStampFirst )
            {
                mTimeStampFirst = timeStamp;
            }
            else if( timeStamp > mTimeStampLast )
            {
                mTimeStampLast = timeStamp;
            }
            imageAdded = true;
            console() << "[sol] Image [" << srcname << "/" << (int)src << "] " << timeStamp << " added (" << texture.getWidth() << "x" << texture.getHeight() << ")" << std::endl;
        }
        else
        {
            console() << "[sol] Image [" << srcname << "] already exists in this frame." << std::endl;
            // sanity check
            for (int i=0; i < SOURCE_COUNT; ++i)
            {
                if( mTexture[i] == gl::Texture() )
                {
                    console() << "[sol] WARNING: frame missing image source " << i << std::endl;
                }
            }
        }
    }
    else
    {
        console() << "[sol] ERROR: unknown source type from string: " << src << std::endl;
    }
    
    return imageAdded;
}

int SolFrame::getImageCount() const
{
    int count = 0;
    for (int i=0; i < SOURCE_COUNT; ++i)
    {
        if( mTexture[i] != gl::Texture() )
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
        case 94:
            return SOURCE_0094;
        case 131:
            return SOURCE_0131;
        case 171:
            return SOURCE_0171;
        case 193:
            return SOURCE_0193;
        case 211:
            return SOURCE_0211;
        case 304:
            return SOURCE_0304;
        case 335:
            return SOURCE_0335;
        case 1600:
            return SOURCE_1600;
        case 1700:
            return SOURCE_1700;
        default:
            return SOURCE_COUNT;
    }
}

ci::gl::Texture SolFrame::getTexture( const SolFrame::eImageSource src ) const
{
    if( src < SOURCE_COUNT )
    {
        return mTexture[src];
    }
    
    return gl::Texture();
}
