//
//  OutputChannel.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#pragma once

#include "cinder/gl/Texture.h"
#include "NamedObject.h"

namespace oculon
{

class OutputChannel : public NamedObject<OutputChannel>
{
public:
    OutputChannel( const std::string& name, int32_t width, int32_t height );
    
    virtual void outputFrame( ci::gl::TextureRef tex ) = 0;
    
    bool isActive() const           { return mActive; }
    void setActive( bool active )   { mActive = active; }
    
    ci::Vec2i getSize() const       { return ci::Vec2i( mWidth, mHeight ); }
    ci::Area getBounds() const      { return ci::Area( 0, 0, mWidth, mHeight ); }
    int32_t getWidth() const        { return mWidth; }
    int32_t getHeight() const       { return mHeight; }
    
private:
    int32_t mWidth;
    int32_t mHeight;
    bool    mActive;
};

class WindowOutputChannel : public OutputChannel
{
public:
    WindowOutputChannel( const std::string& name, int32_t width, int32_t height );
    virtual void outputFrame( ci::gl::TextureRef tex );
};
    
}

