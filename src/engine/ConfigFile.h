//
//  ConfigFile.h
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-11-18.
//
//

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Json.h"

namespace oculon
{
/// JSON config file representation
class ConfigFile
{
public:
    static constexpr const char* kSettings = "settings";
    static constexpr const char* kRender = "render";
    static constexpr const char* kWindow = "window";
    static constexpr const char* kOutput = "output";
    static constexpr const char* kWidth = "width";
    static constexpr const char* kHeight = "height";
    static constexpr const char* kMidi = "midi";
    static constexpr const char* kOsc = "osc";
    static constexpr const char* kEnabled = "enabled";
    
public:
    void load(ci::DataSourceRef source);
    
    ci::Vec2i getWindowSize();
    ci::Vec2i getOutputSize();
    
    bool isMidiEnabled() { return mJsonTree[kSettings][kMidi][kEnabled].getValue<bool>(); }
    
    bool isOscEnabled() { return mJsonTree[kSettings][kOsc][kEnabled].getValue<bool>(); };
    // TODO: multiple OSC interfaces
    // TODO: multiple OSC relays
    //std::pair<std::string, int32_t> getOscConnectionInfo();
    //int_32t getOscIterfacePort();
    //std::string getOscIpAddress();
    
private:
    ci::JsonTree mJsonTree;
};

} // namespace oculon
