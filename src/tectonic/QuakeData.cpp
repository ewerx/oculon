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

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
using namespace boost::posix_time;
using namespace boost;

// ----------------------------------------------------------------
// 
QuakeEvent::QuakeEvent()
: mLat(0.0f)
, mLong(0.0f)
, mMag(0.0f)
, mType(0)
, mTitle("")
{
    
}

// ----------------------------------------------------------------
// 
QuakeEvent::QuakeEvent( float aLat, float aLong, float aMag, float aDepth, int aType, ptime& aTimeStamp, string aTitle )
: mLat(aLat)
, mLong(aLong)
, mMag(aMag)
, mDepth(aDepth)
, mType(aType)
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
    //<< to_simple_string(mTimeStamp) << ","
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
void QuakeData::addEvent( float aLat, float aLong, float aMag, float aDepth, int aType, ptime& aTimeStamp, std::string aTitle )
{
	mQuakeEvents.push_back( QuakeEvent( aLat, aLong, aMag, aDepth, aType, aTimeStamp, aTitle ) );
    //time_t epochTimestamp = Utils::toEpochSeconds(aTimeStamp);
    //mQuakeEvents[epochTimestamp] = QuakeEvent( aLat, aLong, aMag, aDepth, aType, aTimeStamp, aTitle );
}

//
// ----------------------------------------------------------------
// 


// ----------------------------------------------------------------
// 
void USGSQuakeData::parseData( std::string url )
{
    // format: 2012-04-28T14:33:07Z
    time_input_facet* p_facet = new time_input_facet("%Y-%m-%dT%H:%M:%SZ");
    // locale takes ownership of p_facet
    locale timeStampLocale(locale(""), p_facet);
    
    const XmlTree xml( loadFile( url ) );//loadUrl( Url( url ) ) );
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
            
            const int type = 0;
            addEvent( locationVector.x, locationVector.y, magnitude, depth, type, timeStamp, title );
        } 
        catch(std::exception& e) 
        {
            std::cout << "Parsing Exception: " <<  e.what() << std::endl;
        }
	}
	//console() << xml << std::endl;
    console() << "[tectonic] " << mQuakeEvents.size() << " events added" << std::endl;
}

// ----------------------------------------------------------------
//
void NuclearEventData::parseData( std::string url )
{
    // format: M/d/YYYY H:M:S
    time_input_facet* p_facet = new time_input_facet("%m/%d/%Y %H:%M:%S");
    // locale takes ownership of p_facet
    locale timeStampLocale(locale(""), p_facet);
    
    std::string line;
	std::ifstream myfile( url.c_str() );
	
	if( myfile.is_open() )
    {
		int i=0;
		while( !myfile.eof() )
        {
			std::getline( myfile, line );
            if( line[0] != '#' )
            {
                ++i;
                int type = 0;
                float magnitude = 0.0f;
                float depth = 0.0f;
                ptime timeStamp;
                string title = "";
                float lat = 0.0f;
                float lon = 0.0f;
                
                tokenizer< escaped_list_separator<char> > tokens(line);
                std::string name;
                int col = 0;
                //Country,Source Type,Date Time,Latitude,Longitude,Depth,Yield,Name
                BOOST_FOREACH(std::string t, tokens)
                {
                    if( t.length() > 0 )
                    {
                        switch(col)
                        {
                            case 1://Type
                                if( t[0] == 'A' )//Atomspheric
                                {
                                    type = 0;
                                }
                                else//Under...
                                {
                                    if( t[5] == 'g' )
                                    {
                                        type = 1;//Underground
                                    }
                                    else
                                    {
                                        type = 2;//Underwater
                                    }
                                }
                                break;
                                
                            case 2://DateTime
                            {
                                stringstream timestampLine( t );
                                timestampLine.imbue(timeStampLocale);
                                
                                timestampLine >> timeStamp;
                            }
                                break;
                                
                            case 3://Latitude
                                lat = lexical_cast<float>(t);
                                break;
                                
                            case 4://Longitude
                                lon = lexical_cast<float>(t);
                                break;
                                
                            case 5://Depth
                                depth = lexical_cast<float>(t);
                                break;
                                
                            case 6://Yield
                                magnitude = lexical_cast<float>(t);
                                break;
                                
                            case 7://Name
                                title = t;
                                
                            default:
                                break;
                        }
                        
                    }
                    
                    col++;
                }

                addEvent( lat, lon, magnitude, depth, type, timeStamp, title );
            }
        }
	}
        
    console() << "[nuclear] " << mQuakeEvents.size() << " events added" << std::endl;
}
