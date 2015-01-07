//
//  OutputChannel.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#include "OutputChannel.h"

using namespace oculon;
using namespace cinder;
using namespace cinder::gl;
using namespace std;

OutputChannel::OutputChannel( const string& name, int32_t width, int32_t height )
: NamedObject(name)
, mWidth(width)
, mHeight(height)
{
    
}

void WindowOutputChannel::outputFrame(TextureRef tex)
{
    gl::pushMatrices();
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    gl::enableAlphaBlending();
    glEnable(GL_TEXTURE_2D);
    
    gl::color( ColorA::white() );
    gl::setViewport( getBounds() );
    
    // TODO: support locked aspect ratio?
    gl::draw( *tex, getBounds() );
    
    glPopAttrib();
    gl::popMatrices();
}
