//
//  Parameter.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-06.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "OscParam.h"
#include "OscMessage.h"
#include "SimpleGUI.h"

using namespace ci;
using namespace mowa::sgui;

#pragma MARK OscParam

// constructor
OscParam::OscParam( const std::string& recvAddr, const std::string& sendAddr )
: mOscRecvAddress(recvAddr)
, mOscSendAddress(sendAddr)
, mIsSender(sendAddr.length() > 0)
{

}

#pragma MARK OscFloatParam

OscFloatParam::OscFloatParam(FloatVarControl* control, const std::string& recvAddr, const std::string& sendAddr)
: OscParam(recvAddr, sendAddr)
, mControl(control)
{
    assert(mControl != NULL);
}

void OscFloatParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 && osc::TYPE_FLOAT == message.getArgType(0) )
    {
        mControl->setNormalizedValue( message.getArgAsFloat(0) );
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for float parameter" << std::endl;
    }
}

void OscFloatParam::setOscMessage( osc::Message& message )
{
    message.addFloatArg( mControl->getNormalizedValue() );
    message.setAddress( mOscSendAddress );
}

#pragma MARK OscIntParam

OscIntParam::OscIntParam(IntVarControl* control, const std::string& recvAddr, const std::string& sendAddr)
: OscParam(recvAddr, sendAddr)
, mControl(control)
{
    assert(mControl != NULL);
}

void OscIntParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 && osc::TYPE_FLOAT == message.getArgType(0) )
    {
        mControl->setNormalizedValue( message.getArgAsFloat(0) );
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for int parameter" << std::endl;
    }
}

void OscIntParam::setOscMessage( osc::Message& message )
{
    message.addIntArg( mControl->getNormalizedValue() );
    message.setAddress( mOscSendAddress );
}

#pragma MARK OscBoolParam

OscBoolParam::OscBoolParam(BoolVarControl* control, const std::string& recvAddr, const std::string& sendAddr)
: OscParam(recvAddr, sendAddr)
, mControl(control)
{
    assert(mControl != NULL);
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

void OscBoolParam::setOscMessage( osc::Message& message )
{
    message.addIntArg( (*(mControl->var)) ? 1 : 0 );
    message.setAddress( mOscSendAddress );
}
