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

class OscServer;

class OscParam
{
public:
    enum eType
    {
        PARAMTYPE_FLOAT,
        PARAMTYPE_INT,
        PARAMTYPE_BOOL,
        PARAMTYPE_TRIGGER,
        PARAMTYPE_ENUM,
        PARAMTYPE_VECTOR,
        
        PARAMTYPE_COUNT
    };
public:
    OscParam( const eType type, OscServer* server, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback );
    virtual ~OscParam() { }
        
    const std::string& getOscRecvAddress() const    { return mOscRecvAddress; }
    const std::string& getOscSendAddress() const    { return mOscSendAddress; }
    const std::string& getOscCreateCmd() const      { return mOscCreateCmd; }
    bool isSender() const                           { return mIsSender; }
    eType getType() const                           { return mType; }
    
    bool valueChangedCallback();
    
    virtual mowa::sgui::Control* getControl() = 0;
    virtual void sendValue() = 0;
    
protected:
    eType mType;
    bool mIsSender;
    //bool mCreateGui;
    
    std::string mOscRecvAddress;
    std::string mOscSendAddress;
    std::string mOscCreateCmd;

    OscServer*  mOscServer;
};

//----------------------------------------------------------------------------- 
class OscFloatParam : public OscParam
{
public:
    OscFloatParam(OscServer* server, mowa::sgui::FloatVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback);
    
    mowa::sgui::Control* getControl()   { return mControl; }

    void sendValue();
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::FloatVarControl* mControl;
    
};

//-----------------------------------------------------------------------------                              
class OscIntParam : public OscParam
{
public:
    OscIntParam(OscServer* server, mowa::sgui::IntVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void sendValue();
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::IntVarControl* mControl;
    
};

//----------------------------------------------------------------------------- 
class OscBoolParam : public OscParam
{
public:
    OscBoolParam(OscServer* server, mowa::sgui::BoolVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void sendValue();
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::BoolVarControl* mControl;
    
};

//-----------------------------------------------------------------------------
class OscTriggerParam : public OscParam
{
public:
    OscTriggerParam(OscServer* server, mowa::sgui::ButtonControl* guiControl, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void sendValue();
    void handleOscMessage( const osc::Message& message );
    
protected:
    mowa::sgui::ButtonControl* mControl;
};

//-----------------------------------------------------------------------------                              
class OscEnumParam : public OscParam
{
public:
    OscEnumParam(OscServer* server, mowa::sgui::IntVarControl* guiControl, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback, const bool isVertical);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void sendValue();
    void handleOscMessage( const osc::Message& message, int index );
    
protected:
    mowa::sgui::IntVarControl* mControl;
    bool mIsVertical;
};

//-----------------------------------------------------------------------------
template<typename T, unsigned int _size> 
class OscVectorParam : public OscParam
{
    OscVectorParam(OscServer* server, mowa::sgui::VectorVarControl<T,_size>* guiControl, const std::string& recvAddr, const std::string& sendAddr, const bool sendsFeedback);
    
    mowa::sgui::Control* getControl()   { return mControl; }
    
    void sendValue();
    void handleOscMessage( const osc::Message& message, int index );
    
protected:
    mowa::sgui::VectorVarControl<T,_size>* mControl;
};

typedef OscVectorParam<float,2>     OscVec2fParam;
typedef OscVectorParam<float,3>     OscVec3fParam;
typedef OscVectorParam<float,4>     OscVec4fParam;

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
    , _feedback(false)
    , _vertical(false)
    {
        if( var != NULL ) _default = *var;
    }
    
    inline CreateParam& minValue( const T& min ) 
    { _min = min; return *this; }
    
    inline CreateParam& maxValue( const T& max ) 
    { _max = max; return *this; }
    
    inline CreateParam& defaultValue( const T& defaultValue )
    { _default = defaultValue; return *this; }
    
    inline CreateParam& oscReceiver( const std::string& sceneName )
    {
        return oscReceiver( sceneName, _name );
    }
    
    inline CreateParam& oscReceiver( const std::string& sceneName, const std::string& paramName )
    {
        _recvAddr = "/oculon/" + sceneName + '/' + paramName; 
        _sendAddr = _recvAddr; // default to send to same address
        return *this; 
    }
    
    inline CreateParam& oscSender( const std::string& sceneName, const std::string& paramName )
    { 
        _sendAddr = "/oculon/" + sceneName + '/' + paramName; 
        return *this; 
    }
    
    inline CreateParam& oscSender( const std::string& address )
    { _sendAddr = address; return *this; }
    
    inline CreateParam& sendFeedback( const bool feedback =true )
    { _feedback = feedback; return *this; }
    
    inline CreateParam& isVertical( const bool vertical =true )
    { _vertical = vertical; return *this; }
    
protected:
    friend class Interface;
    
    std::string _name;
    T*          _var;
    T           _min;
    T           _max;
    T           _default;
    std::string _sendAddr;
    std::string _recvAddr;
    bool        _feedback;
    bool        _vertical;
};

typedef CreateParam<float>                  CreateFloatParam;
typedef CreateParam<int32_t>                CreateIntParam;
typedef CreateParam<bool>                   CreateBoolParam;
typedef CreateParam<char>                   CreateTriggerParam;
typedef CreateParam<int>                    CreateEnumParam;

