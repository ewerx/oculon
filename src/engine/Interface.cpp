//
//  Interface.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-07.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "Interface.h"
#include "SimpleGUI.h"
#include "OscServer.h"

using namespace std;
using namespace ci;
using namespace mowa::sgui;

// constructor
Interface::Interface(app::App* app, OscServer* osc)
: mGui(NULL)
, mOsc(osc)
{
    mGui = new SimpleGUI(app);
}

Interface::~Interface()
{
    for (vector<OscParam*>::iterator it = mParams.begin(); 
         it != mParams.end();
         ++it )
    {
        OscParam* param = (*it);
        delete param;
    }
    
    delete mGui;
}

OscFloatParam* Interface::addParam( const CreateFloatParam& param )
{
    mowa::sgui::FloatVarControl* control = mGui->addParam( param._name, param._var, param._min, param._max, param._default );
    
    OscFloatParam* oscParam = new OscFloatParam( mOsc, control, param._recvAddr, param._sendAddr );
    mParams.push_back(oscParam);
    
    return oscParam;
}

OscIntParam* Interface::addParam( const CreateIntParam& param )
{
    mowa::sgui::IntVarControl* control = mGui->addParam( param._name, param._var, param._min, param._max, param._default );
    
    OscIntParam* oscParam = new OscIntParam( mOsc, control, param._recvAddr, param._sendAddr );
    mParams.push_back(oscParam);
    
    return oscParam;
}

OscBoolParam* Interface::addParam( const CreateBoolParam& param )
{
    mowa::sgui::BoolVarControl* control = mGui->addParam( param._name, param._var, param._default );
    
    OscBoolParam* oscParam = new OscBoolParam( mOsc, control, param._recvAddr, param._sendAddr );
    mParams.push_back(oscParam);

    return oscParam;
}

void Interface::update()
{
}

void Interface::draw()
{
    assert( mGui != NULL );
    
    mGui->draw();
}
