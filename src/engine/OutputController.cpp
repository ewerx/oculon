//
//  OutputController.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2015-01-06.
//
//

#include "OutputController.h"

using namespace oculon;
using namespace cinder;
using namespace ci::app;

OutputController::OutputController()
{
    mChannels["window"] = OutputChannel::Ref(new WindowOutputChannel("window", getWindowWidth(), getWindowHeight()));
    
    // syphon servers
    
    // movie writer
    
    // frame saver
    
    // oculus rift
    
    
}

void OutputController::outputFrame(ci::gl::TextureRef tex)
{
    for( const auto& pair : mChannels )
    {
        OutputChannel::Ref outputChannel = pair.second;
        
        if (outputChannel->isActive())
        {
            outputChannel->outputFrame(tex);
        }
    }
}
