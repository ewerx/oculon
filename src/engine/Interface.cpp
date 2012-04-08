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
    
    OscFloatParam* oscParam = new OscFloatParam( control, param._recvAddr, param._sendAddr );
    mOsc->registerCallback( param._recvAddr, oscParam, &OscFloatParam::handleOscMessage );
    
    mParams.push_back(oscParam);
    
    return oscParam;
}

OscIntParam* Interface::addParam( const CreateIntParam& param )
{
    mowa::sgui::IntVarControl* control = mGui->addParam( param._name, param._var, param._min, param._max, param._default );
    
    OscIntParam* oscParam = new OscIntParam( control, param._recvAddr, param._sendAddr );
    mOsc->registerCallback( param._recvAddr, oscParam, &OscIntParam::handleOscMessage );
    
    mParams.push_back(oscParam);
    
    return oscParam;
}

OscBoolParam* Interface::addParam( const CreateBoolParam& param )
{
    mowa::sgui::BoolVarControl* control = mGui->addParam( param._name, param._var, param._default );
    
    OscBoolParam* oscParam = new OscBoolParam( control, param._recvAddr, param._sendAddr );
    mOsc->registerCallback( param._recvAddr, oscParam, &OscBoolParam::handleOscMessage );
    
    mParams.push_back(oscParam);
    
    return oscParam;
}

void Interface::update()
{
    // sending all values every frame
    // this may benefit from multi-threading or sending values ad-hoc
    for (vector<OscParam*>::iterator it = mParams.begin(); 
         it != mParams.end();
         ++it )
    {
        OscParam* param = (*it);
        if( param->isSender() )
        {
            osc::Message message;
            param->setOscMessage( message );
            mOsc->sendMessage( message );
        }
    }
    
}

void Interface::draw()
{
    if( mGui )
    {
        mGui->draw();
    }
}