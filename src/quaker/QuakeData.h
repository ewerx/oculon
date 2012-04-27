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

class QuakeEvent
{
public:
    QuakeEvent();
    QuakeEvent( float aLat, float aLong, float aMag, std::string aTitle );
    virtual ~QuakeEvent();
    
    float getLat() const    { return mLat; }
    float getLong() const   { return mLong; }
    float getMag() const    { return mMag; }
    const std::string& getTitle()   { return mTitle; }
    
protected:
    float       mLat;
	float       mLong;
	float       mMag;
	std::string mTitle;
};

//
//
//
class QuakeData
{
public:
    typedef std::vector<QuakeEvent> EventList;
public:
    QuakeData();
    virtual ~QuakeData();
    
    virtual void parseData(std::string url) = 0;
    
    void addEvent( float aLat, float aLong, float aMag, std::string aTitle );
    
    EventList::const_iterator   eventsBegin() const     { return mQuakeEvents.begin(); }
    EventList::const_iterator   eventsEnd() const       { return mQuakeEvents.end(); }
    
protected:
    std::vector<QuakeEvent> mQuakeEvents;
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

