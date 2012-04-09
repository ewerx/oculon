//
//  Parameter.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-06.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "SimpleGUI.h"
#include "OscMessage.h"

class OscParam
{
    
public:
    OscParam( const std::string& recvAddr, const std::string& sendAddr );
    virtual ~OscParam() { }
        
    const std::string& getOscRecvAddress() const    { return mOscRecvAddress; }
    const std::string& getOscSendAddress() const    { return mOscSendAddress; }
    const std::string& getOscCreateCmd() const      { return mOscCreateCmd; }
    bool isSender() const                           { return mIsSender; }
    
    virtual mowa::sgui::Control* getControl() = 0;
    virtual void prepOscSend( osc::Message& message ) = 0;
    virtual void handleOscMessage(const osc::Message& message) = 0;
    
protected:
    bool mIsSender;
    //bool mCreateGui;
    
    std::string mOscRecvAddress;
    std::string mOscSendAddress;
    std::string mOscCreateCmd;

    //TODO: change callback
};

//----------------------------------------------------------------------------- 
class OscFloatParam : public OscParam
{
public:
    OscFloatParam(mowa::sgui::FloatVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr);
    
    mowa::sgui::Control* getControl()   { return mControl; }

    void prepOscSend( osc::Message& message );
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::FloatVarControl* mControl;
    
};

//-----------------------------------------------------------------------------                              
class OscIntParam : public OscParam
{
public:
    OscIntParam(mowa::sgui::IntVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void prepOscSend( osc::Message& message );
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::IntVarControl* mControl;
    
};

//----------------------------------------------------------------------------- 
class OscBoolParam : public OscParam
{
public:
    OscBoolParam(mowa::sgui::BoolVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void prepOscSend( osc::Message& message );
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::BoolVarControl* mControl;
    
};

//-----------------------------------------------------------------------------
class OscTriggerParam : public OscParam
{
public:
    OscTriggerParam(mowa::sgui::ButtonControl* guiControl, const std::string& recvAddr, const std::string& sendAddr);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void prepOscSend( osc::Message& message );
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::ButtonControl* mControl;
};

//-----------------------------------------------------------------------------
class Interface;

template<typename T>
class CreateParam
{
public:
    inline CreateParam( const std::string& name, T* var )
    : _name(name)
    , _var(var)
    , _min(0)
    , _max(1)
    , _default(0)
    , _sendAddr("")
    , _recvAddr("")
    { }
    
    inline CreateParam& minValue( const T& min ) 
    { _min = min; return *this; }
    
    inline CreateParam& maxValue( const T& max ) 
    { _max = max; return *this; }
    
    inline CreateParam& defaultValue( const T& defaultValue )
    { _default = defaultValue; return *this; }
    
    inline CreateParam& oscReceiver( const std::string& address )
    { _recvAddr = address; return *this; }
    
    inline CreateParam& oscSender( const std::string& address )
    { _sendAddr = address; return *this; }
    
private:
    friend class Interface;
    
    std::string _name;
    T*          _var;
    T           _min;
    T           _max;
    T           _default;
    std::string _sendAddr;
    std::string _recvAddr;
};

typedef CreateParam<float>                  CreateFloatParam;
typedef CreateParam<int32_t>                CreateIntParam;
typedef CreateParam<bool>                   CreateBoolParam;
//TODO: trigger param

