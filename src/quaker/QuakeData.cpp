//
//  QuakeData.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-25.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "QuakeData.h"
#include "Utils.h"

#include "cinder/Xml.h"
#include "cinder/Url.h"
#include "cinder/app/AppBasic.h"
using namespace ci;
using namespace ci::app;

#include <vector>
#include <sstream>
using std::locale;
using std::vector;
using std::string;
using std::istringstream;
using std::stringstream;

#include <boost/date_time.hpp>
using namespace boost::posix_time;

// ----------------------------------------------------------------
// 
QuakeEvent::QuakeEvent()
: mLat(0.0f)
, mLong(0.0f)
, mMag(0.0f)
, mTitle("")
{
    
}

// ----------------------------------------------------------------
// 
QuakeEvent::QuakeEvent( float aLat, float aLong, float aMag, float aDepth, ptime& aTimeStamp, string aTitle )
: mLat(aLat)
, mLong(aLong)
, mMag(aMag)
, mDepth(aDepth)
, mTimeStamp(aTimeStamp)
, mTitle(aTitle)
{
}

QuakeEvent::~QuakeEvent()
{
}


std::string QuakeEvent::toString() const
{
    stringstream ss;
    ss << mTitle << "," 
    << mMag << "," 
    << mDepth << "m,"
    << mTimeStamp << ","
    << mLat << "," << mLong;
    
    return ss.str();
}

//
// ----------------------------------------------------------------
// 

// ----------------------------------------------------------------
// 
QuakeData::QuakeData()
{
}

// ----------------------------------------------------------------
// 
QuakeData::~QuakeData()
{
    
}

// ----------------------------------------------------------------
// 
void QuakeData::addEvent( float aLat, float aLong, float aMag, float aDepth, ptime& aTimeStamp, std::string aTitle )
{
	//mQuakeEvents.push_back( QuakeEvent( aLat, aLong, aMag, aDepth, aTimeStamp, aTitle ) );
    time_t epochTimestamp = Utils::toEpochSeconds(aTimeStamp);
    mQuakeEvents[epochTimestamp] = QuakeEvent( aLat, aLong, aMag, aDepth, aTimeStamp, aTitle );
}

//
// ----------------------------------------------------------------
// 


// ----------------------------------------------------------------
// 
void USGSQuakeData::parseData( std::string url )
{
    //const char* url = "http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml";
    // format: 2012-04-28T14:33:07Z
    time_input_facet* p_facet = new time_input_facet("%Y-%m-%dT%H:%M:%SZ");
    // locale takes ownership of p_facet
    locale timeStampLocale(locale(""), p_facet);
    
    const XmlTree xml( loadUrl( Url( url ) ) );
	for( XmlTree::ConstIter itemIter = xml.begin( "feed/entry" ); itemIter != xml.end(); ++itemIter ) 
    {
        try 
        {
            string titleLine( itemIter->getChild( "title" ).getValue() );
            size_t firstComma = titleLine.find( ',' );
            float magnitude = fromString<float>( titleLine.substr( titleLine.find( ' ' ) + 1, firstComma - 2 ) );
            string title = titleLine.substr( firstComma + 2 );
            
            istringstream locationString( itemIter->getChild( "georss:point" ).getValue() );
            Vec2f locationVector;
            locationString >> locationVector.x >> locationVector.y;
            
            string depthLine( itemIter->getChild( "georss:elev" ).getValue() );
            float depth = fromString<float>( depthLine ); // meters
            
            
            stringstream timestampLine( itemIter->getChild( "updated" ).getValue() );
            timestampLine.imbue(timeStampLocale);
            ptime timeStamp;
            timestampLine >> timeStamp;
            
            addEvent( locationVector.x, locationVector.y, magnitude, depth, timeStamp, title );
        } 
        catch(std::exception& e) 
        {
            std::cout << "Parsing Exception: " <<  e.what() << std::endl;
        }
	}
	//console() << xml << std::endl;
    console() << "[quaker] " << mQuakeEvents.size() << " events added" << std::endl;
}
