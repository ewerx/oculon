//
//  Config.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-07-04.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

// this code adapted from FieldKit.cpp (https://github.com/field/FieldKit.cpp) Config class


#include "Config.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Xml.h"
#include "rapidxml/rapidxml.hpp"
#include <boost/algorithm/string.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;

void Config::loadXML(DataSourceRef source)
{
    console() << "[main] loading settings" << std::endl;
    XmlTree doc;
    try {
        doc = XmlTree(source);	
    } catch(rapidxml::parse_error &e) {
        console() << "Config::loadXML - couldnt parse XML data ("<< 
                    "error: "<< e.what() <<" "<<
                    "file: "<< source->getFilePathHint() <<
                    ")" << std::endl;
    }
    
    XmlTree root = *doc.begin();
    
    for(XmlTree::ConstIter setting = root.begin(); setting != root.end(); ++setting) 
    {
        string key = setting->getTag();
        string value = setting->getValue();
        console() << '\t' << key << ": " << value << std::endl;
        mSettings.insert(std::pair<string,string>(key, value) );
    }
}

void Config::overrideWith(const std::vector<std::string>& args)
{
    for(unsigned int i=1; i<args.size(); i++) 
    {
        // check if argument is a key=value pair
        std::string arg = args[i];
        std::vector<std::string> strs;
        boost::split(strs, arg, boost::is_any_of("="));
        if(strs.size() == 2) {
            string key = strs[0];
            // remove leading - if exists
            if(key.substr(0,1) == "-") key = key.substr(1);
            mSettings[key] = strs[1];
        }
    }
}

void Config::overrideWith(int argc, char* const argv[])
{
    std::vector<std::string> args;
    for(int arg = 0; arg < argc; ++arg)
        args.push_back(std::string(argv[arg]));
    
    overrideWith(args);
}

void Config::print()
{
    console() << "Settings:" << std::endl;
    
    tSettingsMap::iterator it;
    for(it=mSettings.begin() ; it != mSettings.end(); it++ )
        console() << "* " << (*it).first << " = " << (*it).second << std::endl;
}


// -- Getters ------------------------------------------------------------------
string Config::getString(const string key, string alt) 
{ 
    return getValue<string>(key, alt); 
}

bool Config::getBool(const string key, bool alt) 
{
    return getValue<bool>(key, alt); 
}


int Config::getInt(const string key, int alt) 
{ 
    return getValue<int>(key, alt); 
}


double Config::getDouble(const string key, double alt) 
{ 
    return getValue<double>(key, alt); 
}


float Config::getFloat(const string key, float alt) 
{ 
    return getValue<float>(key, alt); 
}


Vec2f Config::getVec2f(const string key, Vec2f alt)
{
    string value = getString(key);
    
    std::vector<std::string> parts;
    boost::split(parts, value, boost::is_any_of(","));
    
    if(parts.size() == 2) {
        float x = fromString<float>(parts[0], 0.0f);
        float y = fromString<float>(parts[1], 0.0f);
        return Vec2f(x,y);
        
    } else {
        return alt;
    }
}


Vec3f Config::getVec3f(const string key, Vec3f alt)
{
    string value = getString(key);
    
    std::vector<std::string> parts;
    boost::split(parts, value, boost::is_any_of(","));
    
    if(parts.size() == 3) {
        float x = fromString<float>(parts[0], 0.0f);
        float y = fromString<float>(parts[1], 0.0f);
        float z = fromString<float>(parts[2], 0.0f);
        return Vec3f(x,y,z);
        
    } else {
        return alt;
    }
}

Vec2i Config::getVec2i(const string key, Vec2i alt)
{
    string value = getString(key);
    
    std::vector<std::string> parts;
    boost::split(parts, value, boost::is_any_of(","));
    
    if(parts.size() == 2) {
        float x = fromString<float>(parts[0], 0.0f);
        float y = fromString<float>(parts[1], 0.0f);
        return Vec2i(x,y);
        
    } else {
        return alt;
    }
}