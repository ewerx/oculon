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

using namespace ci;
using namespace mowa::sgui;

#pragma MARK OscParam

// constructor
OscParam::OscParam(OscServer* server, const std::string& recvAddr, const std::string& sendAddr)
: mOscServer(server)
, mOscRecvAddress(recvAddr)
, mOscSendAddress(sendAddr)
, mIsSender(sendAddr.length() > 0)
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

OscFloatParam::OscFloatParam(OscServer* server, FloatVarControl* control, const std::string& recvAddr, const std::string& sendAddr)
: OscParam(server, recvAddr, sendAddr)
, mControl(control)
{
    assert(mControl != NULL);
    server->registerCallback( recvAddr, this, &OscFloatParam::handleOscMessage );
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

OscIntParam::OscIntParam(OscServer* server, IntVarControl* control, const std::string& recvAddr, const std::string& sendAddr)
: OscParam(server, recvAddr, sendAddr)
, mControl(control)
{
    assert(mControl != NULL);
    server->registerCallback( recvAddr, this, &OscIntParam::handleOscMessage );
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscIntParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 && osc::TYPE_FLOAT == message.getArgType(0) )
    {
        mControl->setNormalizedValue( message.getArgAsFloat(0), true );
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

OscBoolParam::OscBoolParam(OscServer* server, BoolVarControl* control, const std::string& recvAddr, const std::string& sendAddr)
: OscParam(server, recvAddr, sendAddr)
, mControl(control)
{
    assert(mControl != NULL);
    server->registerCallback( recvAddr, this, &OscBoolParam::handleOscMessage );
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
                break;
        
            case osc::TYPE_FLOAT:
                *(mControl->var) = (message.getArgAsFloat(0) != 0.0f);
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
