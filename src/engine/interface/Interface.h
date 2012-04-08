//
//  Interface.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-07.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#pragma once

#include <vector>
#include "OscParam.h"

class mowa::sgui::SimpleGUI;
class OscServer;

class Interface
{
public:
    Interface(ci::app::App* app, OscServer* osc);
    virtual ~Interface();
    
    OscFloatParam* addParam( const CreateFloatParam& param );
    
    void update();
    void draw();
    
private:
    mowa::sgui::SimpleGUI* mGui;
    OscServer* mOsc;
    
    std::vector<OscParam*> mParams;
};
