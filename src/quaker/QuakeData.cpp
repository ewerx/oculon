//
//  QuakeData.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-04-25.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "QuakeData.h"

#include "cinder/Xml.h"
#include "cinder/Url.h"
#include "cinder/app/AppBasic.h"
using namespace ci;
using namespace ci::app;

#include <vector>
#include <sstream>
using std::vector;
using std::string;
using std::istringstream;
using std::stringstream;

// ----------------------------------------------------------------
// 
Quake::Quake()
: mLat(0.0f)
, mLong(0.0f)
, mMag(0.0f)
, mTitle("")
{
    
}

// ----------------------------------------------------------------
// 
Quake::~Quake()
{
    
}

// ----------------------------------------------------------------
// 
Quake::Quake( float aLat, float aLong, float aMag, string aTitle )
{
	mLat	= aLat;
	mLong	= aLong;
	mMag	= aMag;
	mTitle	= aTitle;
    
    // calculate globe location
    //float theta = toRadians( 90 - mLat );
    //float phi	= toRadians( 180 - mLong );
    
	//mLoc		= Vec3f( sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi) );
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
void QuakeData::addQuake( float aLat, float aLong, float aMag, std::string aTitle )
{
	mQuakes.push_back( Quake( aLat, aLong, aMag, aTitle ) );
    
    //console() << "Quake" << mQuakes.size() << ": " << aTitle << aMag << aLat << aLong << std::endl;
}

//
// ----------------------------------------------------------------
// 


// ----------------------------------------------------------------
// 
void USGSQuakeData::parseData( std::string url )
{
    //const char* url = "http://earthquake.usgs.gov/earthquakes/catalogs/7day-M2.5.xml";
    
    const XmlTree xml( loadUrl( Url( url ) ) );
	for( XmlTree::ConstIter itemIter = xml.begin( "feed/entry" ); itemIter != xml.end(); ++itemIter ) 
    {
		string titleLine( itemIter->getChild( "title" ).getValue() );
		size_t firstComma = titleLine.find( ',' );
		float magnitude = fromString<float>( titleLine.substr( titleLine.find( ' ' ) + 1, firstComma - 2 ) );
		string title = titleLine.substr( firstComma + 2 );
        
		istringstream locationString( itemIter->getChild( "georss:point" ).getValue() );
		Vec2f locationVector;
		locationString >> locationVector.x >> locationVector.y;
		
		addQuake( locationVector.x, locationVector.y, magnitude, title );		
	}
	//console() << xml << std::endl;
    console() << "[quaker] " << mQuakes.size() << " events added" << std::endl;
}
