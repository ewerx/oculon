//
//  QuakeData.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-25.
//  Copyright (c) 2012 ewerx. All rights reserved.
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
    QuakeEvent( float aLat, float aLong, float aMag, float aDepth, int aType, boost::posix_time::ptime& aTimeStamp, std::string aTitle );
    virtual ~QuakeEvent();
    
    float getLat() const                        { return mLat; }
    float getLong() const                       { return mLong; }
    float getMag() const                        { return mMag; }
    float getDepth() const                      { return mDepth; }
    int   getType() const                       { return mType; }
    boost::posix_time::ptime& getTimeStamp()    { return mTimeStamp; }
    const std::string& getTitle() const         { return mTitle; }
    
    std::string toString() const;
    
protected:
    float       mLat;
	float       mLong;
	float       mMag;
    float       mDepth;
    int         mType;
    boost::posix_time::ptime  mTimeStamp;
    
	std::string mTitle;
};

//
//
//
class QuakeData
{
public:
    typedef std::vector<QuakeEvent> EventMap;
    //typedef std::map<time_t,QuakeEvent> EventMap;
public:
    QuakeData();
    virtual ~QuakeData();
    
    virtual void parseData(std::string url) = 0;
    
    void addEvent( float aLat, float aLong, float aMag, float aDepth, int aType, boost::posix_time::ptime& aTimeStamp, std::string aTitle );
    
    EventMap::const_iterator   eventsBegin() const     { return mQuakeEvents.begin(); }
    EventMap::const_iterator   eventsEnd() const       { return mQuakeEvents.end(); }
    
protected:
    EventMap mQuakeEvents;
};

class USGSQuakeData : public QuakeData
{
public:
    void parseData(std::string url);
    
};

class NuclearEventData : public QuakeData
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

