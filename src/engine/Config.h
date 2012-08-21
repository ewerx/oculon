//
//  Config.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-07-04.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

// this code adapted from FieldKit.cpp (https://github.com/field/FieldKit.cpp) ConfigDict class

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include "cinder/Utilities.h"

#include <map>
#include <string>
#include <boost/lexical_cast.hpp>

//! A simple configuration dictionary
class Config 
{
public:
	void loadXML(ci::DataSourceRef source);
    //! override with cinder-style args
	void overrideWith(const std::vector<std::string>& args);
    //! override with c style args
    void overrideWith(int argc, char * const argv[]);
	void print();
    
    std::string getString(const std::string key, std::string alt="") const;
	bool getBool(const std::string key, bool alt=false) const;
	int getInt(const std::string key, int alt=0) const;
	float getFloat(const std::string key, float alt=0.0f) const;
	double getDouble(const std::string key, double alt=0.0) const;
    
    ci::Vec2f getVec2f(const std::string key, ci::Vec2f alt=ci::Vec2f::zero()) const;
    ci::Vec3f getVec3f(const std::string key, ci::Vec3f alt=ci::Vec3f::zero()) const;
    ci::Vec2i getVec2i(const std::string key, ci::Vec2i alt=ci::Vec2i::zero()) const;
    
private:
    typedef std::map<std::string, std::string> tSettingsMap;
    std::map<std::string, std::string> mSettings;
    
    template <typename T>
	T getValue(const std::string key, const T defaultValue) const
	{
		tSettingsMap::const_iterator it;
		it = mSettings.find(key);
		if(it != mSettings.end()) {
			return fromString<T>((*it).second, defaultValue);
		} else {
			return defaultValue;
		}
	};
    
	template <typename T>
	T fromString(const std::string value, const T defaultValue) const 
	{
		try {
			return boost::lexical_cast<T>(value);
		} catch(boost::bad_lexical_cast &e) {
			return defaultValue;
		}
	}
};
