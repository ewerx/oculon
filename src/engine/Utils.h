//
//  Utils.h
//  Oculon
//
//  Created by Ehsan Rezaie on 12-02-08.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#ifndef __OCULON_UTILS_H
#define __OCULON_UTILS_H

#include <string>
#include "cinder/Filesystem.h"
#include "cinder/Vector.h"
#include <boost/date_time.hpp>

class Utils
{
public:
    //! Returns a unique pathname, numbered sequentially to avoid conflicts with files of the same/similar name.	
    static ci::fs::path getUniquePath( const std::string &path, int padding =3, const std::string &sep = "_" );

    //! Returns a left-padded string based on the input string
	static std::string leftPaddedString( const std::string &input, int padding=3, const std::string pad="0" );
    
    // math
    static double randDouble()  { return (double(rand()) / RAND_MAX); }
    
    // map
    static ci::Vec2f toMercatorProjection( const float aLat, const float aLong, const float aMapWidth, const float aMapHeight );
    static ci::Vec2f toEquirectProjection( const float aLat, const float aLong, const float aMapWidth, const float aMapHeight );
    
    static ci::Vec3f toGlobePosition( const float alat, const float aLong );
    
    static time_t toEpochSeconds(boost::posix_time::ptime& t);
};

#endif
