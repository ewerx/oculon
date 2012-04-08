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
    if( message.getNumArgs() == 1 ) 
    {
        if( osc::TYPE_FLOAT == message.getArgType(0) )
        {
            mControl->setNormalizedValue( message.getArgAsFloat(0) );
        }
    }
}

void OscFloatParam::setOscMessage( osc::Message& message )
{
    message.addFloatArg( mControl->getNormalizedValue() );
    message.setAddress( mOscSendAddress );
}
