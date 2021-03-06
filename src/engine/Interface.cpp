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
#include "cinder/Json.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mowa::sgui;

#pragma MARK Controls

mowa::sgui::FloatVarControl* Interface::addParam( const CreateFloatParam& param )
{
    mowa::sgui::FloatVarControl* control = mGui->addParam( param._name, param._var, param._min, param._max, *(param._var) );
    
    OscFloatParam* oscParam = new OscFloatParam( mOsc, control, param );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::IntVarControl* Interface::addParam( const CreateIntParam& param )
{
    mowa::sgui::IntVarControl* control = mGui->addParam( param._name, param._var, param._min, param._max, *(param._var) );
    
    OscIntParam* oscParam = new OscIntParam( mOsc, control, param );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::BoolVarControl* Interface::addParam( const CreateBoolParam& param )
{
    mowa::sgui::BoolVarControl* control = mGui->addParam( param._name, param._var, *(param._var) );
    
    OscBoolParam* oscParam = new OscBoolParam( mOsc, control, param );
    mParams.push_back(oscParam);

    return control;
}

mowa::sgui::ButtonControl* Interface::addButton( const CreateTriggerParam& param )
{
    mowa::sgui::ButtonControl* control = mGui->addButton( param._name );
    
    OscTriggerParam* oscParam = new OscTriggerParam( mOsc, control, param );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::IntVarControl* Interface::addEnum( const CreateEnumParam& param )
{
    mowa::sgui::IntVarControl* control = mGui->addParam( param._name, param._var, param._min, param._max-1, *(param._var) );
    
    OscEnumParam* oscParam = new OscEnumParam( mOsc, control, param );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::EnumVarControl* Interface::addEnum( const CreateEnumParam& param, std::vector<std::string>& enumNames )
{
    mowa::sgui::EnumVarControl* control = mGui->addParam( param._name, param._var, enumNames );
    
    OscEnumParam* oscParam = new OscEnumParam( mOsc, control, param );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::ColorVarControl* Interface::addParam( const CreateColorParam& param )
{
    mowa::sgui::ColorVarControl* control = mGui->addParam( param._name, param._var );
    
    OscColorParam* oscParam = new OscColorParam( mOsc, control, param._recvAddr, param._sendAddr , param._feedback, param._altstyle );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::Vec2fVarControl* Interface::addParam( const CreateVec2fParam& param )
{
    mowa::sgui::Vec2fVarControl* control = mGui->addParam( param._name, param._var->ptr(), param._min, param._max );
    
    OscVec2fParam* oscParam = new OscVec2fParam( mOsc, control, param._recvAddr, param._sendAddr , param._feedback, param._altstyle );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::Vec3fVarControl* Interface::addParam( const CreateVec3fParam& param )
{
    mowa::sgui::Vec3fVarControl* control = mGui->addParam( param._name, param._var->ptr(), param._min, param._max );
    
    OscVec3fParam* oscParam = new OscVec3fParam( mOsc, control, param._recvAddr, param._sendAddr , param._feedback, param._altstyle );
    mParams.push_back(oscParam);
    
    return control;
}

mowa::sgui::Vec4fVarControl* Interface::addParam( const CreateVec4fParam& param )
{
    mowa::sgui::Vec4fVarControl* control = mGui->addParam( param._name, param._var->ptr(), param._min, param._max );
    
    OscVec4fParam* oscParam = new OscVec4fParam( mOsc, control, param._recvAddr, param._sendAddr , param._feedback, param._altstyle );
    mParams.push_back(oscParam);
    
    return control;
}

#pragma MARK Interface

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

void Interface::update()
{
}

void Interface::draw()
{
    assert( mGui != NULL );
    
    glDisable( GL_TEXTURE_2D );
    gl::enableAlphaBlending();
    gl::enableDepthRead();
	gl::enableDepthWrite();
    
    mGui->draw();
}

void Interface::sendAll()
{
    for (vector<OscParam*>::iterator it = mParams.begin(); 
         it != mParams.end();
         ++it )
    {
        OscParam* param = (*it);
        param->sendValue();
    }
}

void Interface::createControlInterface(const std::string& sceneName)
{
    // /control/createBlankInterface "name" "portrait/landscape"
    // /control/pushDestination "192.168.1.140:8889" // use value from oscserver
    osc::Message message;
    message.setAddress("/control/createBlankInterface");
    message.addStringArg(sceneName);
    message.addStringArg("landscape");
    mOsc->sendMessage(message);

    message.clear();
    message.setAddress("/control/pushDestination");
    message.addStringArg("192.168.1.140:8889");
    mOsc->sendMessage(message);
    // ex: /control/addWidget "{'name':'test1', 'type':'Slider', 'bounds':[.0,.0,.75,.3], }"
    // /control/setBounds "test1" 0 0 .2 .3 (floats) 
    // scene switch tabs
    // active/visible/debug
    
    const std::string baseAddress = "/" + sceneName + "/";
    
    const float width = 0.3f;
    const float height = 0.25f;
    
    float x = 0.1f;
    float y = 0.0f;
    
    // params
    for (vector<OscParam*>::iterator it = mParams.begin(); 
         it != mParams.end();
         ++it )
    {
        OscParam* param = (*it);
        
        JsonTree labelJson;
        labelJson.pushBack( JsonTree("name", param->getControl()->name + "_label") );
        labelJson.pushBack( JsonTree("value", param->getControl()->name) );
        labelJson.pushBack( JsonTree("x", x) );
        labelJson.pushBack( JsonTree("y", y) );
        labelJson.pushBack( JsonTree("width", width) );
        labelJson.pushBack( JsonTree("height", height) );
        
        JsonTree widgetJson;
        widgetJson.pushBack( JsonTree("name", param->getControl()->name) );
        widgetJson.pushBack( JsonTree("x", x) );
        widgetJson.pushBack( JsonTree("y", y) );
        widgetJson.pushBack( JsonTree("width", width) );
        widgetJson.pushBack( JsonTree("height", height) );
        
        widgetJson.pushBack( JsonTree("address", baseAddress + param->getControl()->name) );
        
        switch( param->getType() )
        {
            // toggle button
            case OscParam::PARAMTYPE_BOOL:
                widgetJson.pushBack( JsonTree("type", "Button") );
                //json.pushBack( JsonTree("mode", "toggle") );
                break;
                
            // slider
            case OscParam::PARAMTYPE_INT:
            case OscParam::PARAMTYPE_FLOAT:
                widgetJson.pushBack( JsonTree("type", "Slider") );
                break;
                
            //TODO:
            // trigger
            // enum
                
            default:
                break;
        }
        
        y += height;
        if( y >= 1.0f )
        {
            x += width;
            y = 0.0f;
            
            if( x >= 1.0f )
            {
                console() << "[interface] too many params" << std::endl;
                break;
            }
        }
        
        message.clear();
        message.setAddress("/control/addWidget");
        message.addStringArg(widgetJson.serialize());
        mOsc->sendMessage(message);
        message.clear();
        message.setAddress("/control/addWidget");
        message.addStringArg(labelJson.serialize());
        mOsc->sendMessage(message);
        //message.addStringArg(labelJson.serialize());
    }
    
    
    // debug info
    // toggle
    // FPS
}
