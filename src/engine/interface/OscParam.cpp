//
//  Parameter.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-06.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "OscParam.h"
#include "OscMessage.h"
#include "OscServer.h"
#include "SimpleGUI.h"
#include <sstream>

using namespace ci;
using namespace mowa::sgui;
using std::ostringstream;

#pragma MARK OscParam

// constructor
OscParam::OscParam(const eType type, OscServer* server, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback)
: mType(type)
, mOscServer(server)
, mOscRecvAddress(recvAddr)
, mOscSendAddress(sendAddr)
, mIsSender(sendsFeedback)
{
}

bool OscParam::valueChangedCallback()
{
    if( mIsSender )
    {
        osc::Message message;
        prepOscSend( message );
        message.setAddress( mOscSendAddress );
        mOscServer->sendMessage( message );
    }
    
    return false;
}

#pragma MARK OscFloatParam

OscFloatParam::OscFloatParam(OscServer* server, FloatVarControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback)
: OscParam(OscParam::PARAMTYPE_FLOAT, server, recvAddr, sendAddr, sendsFeedback)
, mControl(control)
{
    assert(mControl != NULL);
    if( recvAddr.length() > 0 )
    {
        server->registerCallback( recvAddr, this, &OscFloatParam::handleOscMessage );
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscFloatParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 && osc::TYPE_FLOAT == message.getArgType(0) )
    {
        mControl->setNormalizedValue( message.getArgAsFloat(0), true );
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for float parameter" << std::endl;
    }
}

void OscFloatParam::prepOscSend( osc::Message& message )
{
    message.addFloatArg( mControl->getNormalizedValue() );
}

#pragma MARK OscIntParam

OscIntParam::OscIntParam(OscServer* server, IntVarControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback)
: OscParam(OscParam::PARAMTYPE_INT, server, recvAddr, sendAddr, sendsFeedback)
, mControl(control)
{
    assert(mControl != NULL);
    if( recvAddr.length() > 0 )
    {
        server->registerCallback( recvAddr, this, &OscIntParam::handleOscMessage );
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscIntParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 )
    {
        switch( message.getArgType(0) )
        {
            case osc::TYPE_INT32:
                mControl->setNormalizedValue( message.getArgAsInt32(0), /*silent*/true );
                break;
                
            case osc::TYPE_FLOAT:
                mControl->setNormalizedValue( message.getArgAsFloat(0), /*silent*/true );
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for bool parameter" << std::endl;
                break;
        }
        
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for int parameter" << std::endl;
    }
}

void OscIntParam::prepOscSend( osc::Message& message )
{
    message.addFloatArg( mControl->getNormalizedValue() );
}

#pragma MARK OscBoolParam

OscBoolParam::OscBoolParam(OscServer* server, BoolVarControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback)
: OscParam(OscParam::PARAMTYPE_BOOL, server, recvAddr, sendAddr, sendsFeedback)
, mControl(control)
{
    assert(mControl != NULL);
    if( recvAddr.length() > 0 )
    {
        server->registerCallback( recvAddr, this, &OscBoolParam::handleOscMessage );
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscBoolParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 ) 
    {
        switch( message.getArgType(0) )
        {
            case osc::TYPE_INT32:
                *(mControl->var) = (message.getArgAsInt32(0) != 0);
                mControl->triggerCallback();
                break;
        
            case osc::TYPE_FLOAT:
                *(mControl->var) = (message.getArgAsFloat(0) != 0.0f);
                mControl->triggerCallback();
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for bool parameter" << std::endl;
                break;
        }
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for bool parameter" << std::endl;
    }
}

void OscBoolParam::prepOscSend( osc::Message& message )
{
    message.addIntArg( (*(mControl->var)) ? 1 : 0 );
}

#pragma MARK OscTriggerParam

OscTriggerParam::OscTriggerParam(OscServer* server, ButtonControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback)
: OscParam(OscParam::PARAMTYPE_TRIGGER, server, recvAddr, sendAddr, sendsFeedback)
, mControl(control)
{
    assert(mControl != NULL);
    if( recvAddr.length() > 0 )
    {
        server->registerCallback( recvAddr, this, &OscTriggerParam::handleOscMessage );
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscTriggerParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 ) 
    {
        switch( message.getArgType(0) )
        {
            case osc::TYPE_INT32:
                if( message.getArgAsInt32(0) > 0 )
                {
                    mControl->triggerCallback();
                }
                break;
                
            case osc::TYPE_FLOAT:
                if( message.getArgAsFloat(0) > 0.0f )
                {
                    mControl->triggerCallback();
                }
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for trigger parameter" << std::endl;
                break;
        }
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for trigger parameter" << std::endl;
    }
}

void OscTriggerParam::prepOscSend( osc::Message& message )
{
    message.addIntArg( 1 );
}

#pragma MARK OscEnumParam

OscEnumParam::OscEnumParam(OscServer* server, IntVarControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback, const bool isVertical)
: OscParam(OscParam::PARAMTYPE_ENUM, server, recvAddr, sendAddr, sendsFeedback)
, mControl(control)
, mIsVertical(isVertical)
{
    assert(mControl != NULL);
    
    if( recvAddr.length() > 0 )
    {
        const int enum_count = control->max + 1; // gui max is count-1
        char buf[OSC_ADDRESS_SIZE];
        for( int i = 0; i < enum_count; ++i )
        {
            // enum index is 0-based, osc address is 1-based
            const int row = isVertical ? (enum_count - i) : 1;
            const int col = isVertical ? 1 : i+1;
            snprintf( buf, OSC_ADDRESS_SIZE, "%s/%d/%d", recvAddr.c_str(), row, col );
            server->registerCallback( buf, boost::bind( &OscEnumParam::handleOscMessage, this, _1, i) );
        }
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscEnumParam::handleOscMessage( const osc::Message& message, int index )
{
    if( message.getNumArgs() == 1 )
    {
        switch( message.getArgType(0) )
        {
            case osc::TYPE_FLOAT:
                if( message.getArgAsFloat(0) == 1.0f )
                {
                    console() << "[osc] multi-toggle mapped to index: " << index << std::endl;
                    *(mControl->var) = index;
                }
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for bool parameter" << std::endl;
                break;
        }
        
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for int parameter" << std::endl;
    }
}

void OscEnumParam::prepOscSend( osc::Message& message )
{
    message.addIntArg( mIsVertical ? ((mControl->max+1) - (*(mControl->var))) : 1 );
    message.addIntArg( mIsVertical ? 1 : (*(mControl->var))+1 );
    message.addIntArg( 1 );
}


