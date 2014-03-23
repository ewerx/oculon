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

namespace mowa {
    namespace sgui {
        class SimpleGUI;
    };
};
class OscServer;

class Interface
{
public:
    typedef std::vector<OscParam*>::const_iterator const_iterator;
public:
    Interface(ci::app::App* app, OscServer* osc);
    virtual ~Interface();
    
    mowa::sgui::SimpleGUI*  gui() { return mGui; }
    
    mowa::sgui::FloatVarControl*  addParam( const CreateFloatParam& param );
    mowa::sgui::IntVarControl*    addParam( const CreateIntParam& param );
    mowa::sgui::BoolVarControl*   addParam( const CreateBoolParam& param );
    mowa::sgui::ButtonControl*    addButton( const CreateTriggerParam& param );
    mowa::sgui::IntVarControl*    addEnum( const CreateEnumParam& param );
    mowa::sgui::EnumVarControl*    addEnum( const CreateEnumParam& param, std::vector<std::string>& options );
    mowa::sgui::ColorVarControl*  addParam( const CreateColorParam& param );
    mowa::sgui::Vec2fVarControl*  addParam( const CreateVec2fParam& param );
    mowa::sgui::Vec3fVarControl*  addParam( const CreateVec3fParam& param );
    mowa::sgui::Vec4fVarControl*  addParam( const CreateVec4fParam& param );
    
    void update();
    void draw();
    
    void createControlInterface(const std::string& pageName);
    void sendAll();
    
    const_iterator paramsBegin()    { return mParams.begin(); }
    const_iterator paramsEnd()      { return mParams.end(); }
    
private:
    mowa::sgui::SimpleGUI* mGui;
    OscServer* mOsc;
    
    std::vector<OscParam*> mParams;
};
