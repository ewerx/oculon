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
OscParam::OscParam(const eType type, OscServer* server, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback, const int midiPort, const int midiChannel, const int midiNote)
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
        sendValue();
    }
    
    return false;
}

#pragma MARK OscFloatParam

OscFloatParam::OscFloatParam(OscServer* server, FloatVarControl* control, const CreateFloatParam& param)
: OscParam(OscParam::PARAMTYPE_FLOAT, server, param._recvAddr, param._sendAddr, param._feedback, param._midiPort, param._midiChannel, param._midiNote)
, mControl(control)
, mIsEncoder(param._altstyle)
, mStep(param._step)
{
    assert(mControl != NULL);
    if( param._recvAddr.length() > 0 )
    {
        server->registerCallback( param._recvAddr, this, &OscFloatParam::handleOscMessage );
    }
    if( param._midiChannel != -1 && param._midiNote != -1 )
    {
        server->registerMidiCallback( std::make_pair(param._midiChannel, param._midiNote), this, &OscFloatParam::handleOscMessage );
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscFloatParam::handleOscMessage( const osc::Message& message )
{
    if( message.getNumArgs() == 1 && osc::TYPE_FLOAT == message.getArgType(0) )
    {
        if( mIsEncoder )
        {
            *(mControl->var) += mStep * (message.getArgAsFloat(0) > 0.0f ? 1.0f : -1.0f);
            mControl->triggerCallback();
        }
        else
        {
            mControl->setNormalizedValue( message.getArgAsFloat(0), !mIsSender );
        }
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for float parameter" << std::endl;
    }
}

void OscFloatParam::sendValue()
{
    if( mOscSendAddress.empty() )
        return;
    
    osc::Message message;
    message.setAddress( mOscSendAddress );
    message.addFloatArg( mControl->getNormalizedValue() );
    mOscServer->sendMessage( message );
    
    message.clear();
    message.setAddress( mOscSendAddress + "/val" );
    message.addFloatArg( *(mControl->var) );
    mOscServer->sendMessage( message );
}

#pragma MARK OscIntParam

OscIntParam::OscIntParam(OscServer* server, IntVarControl* control, const CreateIntParam& param)
: OscParam(OscParam::PARAMTYPE_INT, server, param._recvAddr, param._sendAddr, param._feedback, param._midiPort, param._midiChannel, param._midiNote)
, mControl(control)
, mIsEncoder(param._altstyle)
, mStep(param._step)
{
    assert(mControl != NULL);
    if( param._recvAddr.length() > 0 )
    {
        server->registerCallback( param._recvAddr, this, &OscIntParam::handleOscMessage );
    }
    if( param._midiChannel != -1 && param._midiNote != -1 )
    {
        server->registerMidiCallback( std::make_pair(param._midiChannel, param._midiNote), this, &OscIntParam::handleOscMessage );
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
                mControl->setNormalizedValue( message.getArgAsInt32(0), /*silent*/!mIsSender );
                break;
                
            case osc::TYPE_FLOAT:
                if( mIsEncoder )
                {
                    *(mControl->var) += mStep * (message.getArgAsFloat(0) > 0.0f ? 1 : -1);
                    mControl->triggerCallback();
                }
                else
                {
                    mControl->setNormalizedValue( message.getArgAsFloat(0), /*silent*/!mIsSender );
                }
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for int parameter" << std::endl;
                break;
        }
        
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for int parameter" << std::endl;
    }
}

void OscIntParam::sendValue()
{
    if( mOscSendAddress.empty() )
        return;
    
    osc::Message message;
    message.setAddress( mOscSendAddress );
    message.addFloatArg( mControl->getNormalizedValue() );
    mOscServer->sendMessage( message );
    
    message.clear();
    message.setAddress( mOscSendAddress + "/val" );
    message.addFloatArg( *(mControl->var) );
    mOscServer->sendMessage( message );
}

#pragma MARK OscBoolParam

OscBoolParam::OscBoolParam(OscServer* server, BoolVarControl* control, const CreateBoolParam& param)
: OscParam(OscParam::PARAMTYPE_BOOL, server, param._recvAddr, param._sendAddr, param._feedback, param._midiPort, param._midiChannel, param._midiNote)
, mControl(control)
{
    assert(mControl != NULL);
    if( param._recvAddr.length() > 0 )
    {
        server->registerCallback( param._recvAddr, this, &OscBoolParam::handleOscMessage );
    }
    if( param._midiChannel != -1 && param._midiNote != -1 )
    {
        server->registerMidiCallback( std::make_pair(param._midiChannel, param._midiNote), this, &OscBoolParam::handleOscMessage );
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

void OscBoolParam::sendValue()
{
    if( mOscSendAddress.empty() )
        return;
    
    osc::Message message;
    message.setAddress( mOscSendAddress );
    message.addIntArg( (*(mControl->var)) ? 1 : 0 );
    mOscServer->sendMessage( message );
}

#pragma MARK OscTriggerParam

OscTriggerParam::OscTriggerParam(OscServer* server, ButtonControl* control, const CreateTriggerParam& param)
: OscParam(OscParam::PARAMTYPE_TRIGGER, server, param._recvAddr, param._sendAddr, param._feedback, param._midiPort, param._midiChannel, param._midiNote)
, mControl(control)
{
    assert(mControl != NULL);
    if( param._recvAddr.length() > 0 )
    {
        server->registerCallback( param._recvAddr, this, &OscTriggerParam::handleOscMessage );
    }
    if( param._midiChannel != -1 && param._midiNote != -1 )
    {
        server->registerMidiCallback( std::make_pair(param._midiChannel, param._midiNote), this, &OscTriggerParam::handleOscMessage );
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

void OscTriggerParam::sendValue()
{
    //osc::Message message;
    //message.setAddress( mOscSendAddress );
    //message.addIntArg( 1 );
    //mOscServer->sendMessage( message );
}

#pragma MARK OscEnumParam

OscEnumParam::OscEnumParam(OscServer* server, IntVarControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback, const bool isVertical)
: OscParam(OscParam::PARAMTYPE_ENUM, server, recvAddr, sendAddr, sendsFeedback, -1, -1, -1)
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
                console() << "[osc] WARNING: unexpected message format for enum parameter" << std::endl;
                break;
        }
        
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for enum parameter" << std::endl;
    }
}

void OscEnumParam::sendValue()
{
    if( mOscSendAddress.empty() )
        return;
    
    osc::Message message;
    message.setAddress( mOscSendAddress );
    message.addIntArg( mIsVertical ? ((mControl->max+1) - (*(mControl->var))) : 1 );
    message.addIntArg( mIsVertical ? 1 : (*(mControl->var))+1 );
    message.addIntArg( 1 );
    mOscServer->sendMessage( message );
    
}

#pragma MARK OscColorParam

OscColorParam::OscColorParam(OscServer* server, ColorVarControl* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback, const bool isGrouped)
: OscParam(OscParam::PARAMTYPE_COLOR, server, recvAddr, sendAddr, sendsFeedback, -1, -1, -1)
, mControl(control)
, mIsGrouped(isGrouped)
{
    assert(mControl != NULL);
    
    if( recvAddr.length() > 0 )
    {
        char buf[OSC_ADDRESS_SIZE];
        for( int i = 0; i < 4; ++i )
        {
            // osc address is 1-based
            snprintf( buf, OSC_ADDRESS_SIZE, "%s/%d", recvAddr.c_str(), i+1 );
            server->registerCallback( buf, boost::bind( &OscColorParam::handleOscMessage, this, _1, i+1) );
        }
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

void OscColorParam::handleOscMessage( const osc::Message& message, int index )
{
    if( message.getNumArgs() == 1 )
    {
        switch( message.getArgType(0) )
        {
            case osc::TYPE_FLOAT:
                mControl->setValueForElement( index, message.getArgAsFloat(0), true );
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for enum parameter" << std::endl;
                break;
        }
        
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for enum parameter" << std::endl;
    }
}

void OscColorParam::sendValue()
{
    if( mOscSendAddress.empty() )
        return;
    
    if( mIsGrouped )
    {
        osc::Message message;
        message.setAddress( mOscSendAddress );
        for( int i = 1; i <= 4; ++i )
        {
            message.addFloatArg( mControl->getValueForElement(i) );
        }
        mOscServer->sendMessage( message );
    }
    else
    {
        for( int i = 1; i <= 4; ++i )
        {
            osc::Message message;
            ostringstream ss;
            ss << mOscSendAddress << "/" << i;
            message.setAddress( ss.str() );
            message.addFloatArg( mControl->getValueForElement(i) );
            mOscServer->sendMessage( message );
        }
    }
}

#pragma MARK OscVectorParam

template <typename T, unsigned int _size>
OscVectorParam<T,_size>::OscVectorParam(OscServer* server, VectorVarControl<T,_size>* control, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback, const bool isGrouped)
: OscParam(OscParam::PARAMTYPE_VECTOR, server, recvAddr, sendAddr, sendsFeedback, -1, -1, -1)
, mControl(control)
, mIsGrouped(isGrouped)
{
    assert(mControl != NULL);
    
    if( recvAddr.length() > 0 )
    {
        if( mIsGrouped && _size == 2 )
        {
            // special case for XY pad
            server->registerCallback( recvAddr, boost::bind( &OscVectorParam::handleOscMessage, this, _1, 0) );
        }
        else
        {
            char buf[OSC_ADDRESS_SIZE];
            for( int i = 0; i < _size; ++i )
            {
                // osc address is 1-based
                snprintf( buf, OSC_ADDRESS_SIZE, "%s/%d", recvAddr.c_str(), i+1 );
                server->registerCallback( buf, boost::bind( &OscVectorParam::handleOscMessage, this, _1, i) );
            }
        }
    }
    mControl->registerCallback( (OscParam*)(this), &OscParam::valueChangedCallback );
}

template <typename T, unsigned int _size>
void OscVectorParam<T,_size>::handleOscMessage( const osc::Message& message, int index )
{
    if( message.getNumArgs() == 1 )
    {
        switch( message.getArgType(0) )
        {
            case osc::TYPE_FLOAT:
                mControl->setNormalizedValue( index, message.getArgAsFloat(0), true );
                break;
                
            default:
                console() << "[osc] WARNING: unexpected message format for vector parameter" << std::endl;
                break;
        }
        
    }
    else if( mIsGrouped && message.getNumArgs() > 1 )
    {
        // XY pad 
        for (int i=0; i < message.getNumArgs(); ++i)
        {
            switch( message.getArgType(i) )
            {
                case osc::TYPE_FLOAT:
                    mControl->setNormalizedValue( (message.getNumArgs()-1)-i, message.getArgAsFloat(i), true );
                    break;
                    
                default:
                    console() << "[osc] WARNING: unexpected message format for vector parameter" << std::endl;
                    break;
            }
        }
    }
    else
    {
        console() << "[osc] WARNING: unexpected message format for int parameter" << std::endl;
    }
}

template <typename T, unsigned int _size>
void OscVectorParam<T,_size>::sendValue()
{
    if( mOscSendAddress.empty() )
        return;
    
    if( mIsGrouped )
    {
        osc::Message message;
        message.setAddress( mOscSendAddress );
        for( int i = 0; i < _size; ++i )
        {
            message.addFloatArg( mControl->getNormalizedValue(i) );
        }
        mOscServer->sendMessage( message );
    }
    else
    {
        for( int i = 0; i < _size; ++i )
        {
            osc::Message message;
            ostringstream ss;
            ss << mOscSendAddress << "/" << (i+1);
            message.setAddress( ss.str() );
            message.addFloatArg( mControl->getNormalizedValue(i) );
            mOscServer->sendMessage( message );
        }
    }
}

// needed for definining template base class functions in cpp
// see: http://stackoverflow.com/questions/312115/c-linking-errors-undefined-symbols-using-a-templated-class
template class OscVectorParam<float,2>;
template class OscVectorParam<float,3>;
template class OscVectorParam<float,4>;
