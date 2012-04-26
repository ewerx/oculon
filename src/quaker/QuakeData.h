//
//  QuakeData.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-25.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "Cinder/Vector.h"
#include <vector>
#include <string>

class Quake
{
public:
    Quake();
    Quake( float aLat, float aLong, float aMag, std::string aTitle );
    virtual ~Quake();
    
protected:
    float       mLat;
	float       mLong;
	float       mMag;
	std::string mTitle;
    ci::Vec3f   mLoc;
};

//
//
//
class QuakeData
{
public:
    QuakeData();
    virtual ~QuakeData();
    
    virtual void parseData(std::string url) = 0;
    
    void addQuake( float aLat, float aLong, float aMag, std::string aTitle );
    
protected:
    std::vector<Quake> mQuakes;
};

class USGSQuakeData : public QuakeData
{
public:
    void parseData(std::string url);
    
};

//
//
//
//class TabbedQuakeData : public QuakeData
//{
//public:
//    void parseData();
//};

