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
#include <boost/date_time.hpp>

class QuakeEvent
{
public:
    QuakeEvent();
    QuakeEvent( float aLat, float aLong, float aMag, float aDepth, boost::posix_time::ptime& aTimeStamp, std::string aTitle );
    virtual ~QuakeEvent();
    
    float getLat() const    { return mLat; }
    float getLong() const   { return mLong; }
    float getMag() const    { return mMag; }
    const std::string& getTitle()   { return mTitle; }
    std::string toString() const;
    
protected:
    float       mLat;
	float       mLong;
	float       mMag;
    float       mDepth;
    boost::posix_time::ptime  mTimeStamp;
    
	std::string mTitle;
};

//
//
//
class QuakeData
{
public:
    //typedef std::vector<QuakeEvent> EventList;
    typedef std::map<time_t,QuakeEvent> EventMap;
public:
    QuakeData();
    virtual ~QuakeData();
    
    virtual void parseData(std::string url) = 0;
    
    void addEvent( float aLat, float aLong, float aMag, float aDepth, boost::posix_time::ptime& aTimeStamp, std::string aTitle );
    
    EventMap::const_iterator   eventsBegin() const     { return mQuakeEvents.begin(); }
    EventMap::const_iterator   eventsEnd() const       { return mQuakeEvents.end(); }
    
protected:
    //std::vector<QuakeEvent> mQuakeEvents;
    EventMap mQuakeEvents;
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

