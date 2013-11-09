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
#include "cinder/Color.h"
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
    static inline float invSqrt(float x)
    {
        float xhalf = 0.5f * x;
        int i = *(int*)&x; // store floating-point bits in integer
        i = 0x5f3759d5 - (i >> 1); // initial guess for Newton's method
        x = *(float*)&i; // convert new bits into float
        x = x*(1.5f - xhalf*x*x); // One round of Newton's method
        return x;
    }
    
    // map
    static ci::Vec2f toMercatorProjection( const float aLat, const float aLong, const float aMapWidth, const float aMapHeight );
    static ci::Vec2f toEquirectProjection( const float aLat, const float aLong, const float aMapWidth, const float aMapHeight, const int aMapOffset );
    
    static ci::Vec3f toGlobePosition( const float alat, const float aLong );
    
    static time_t toEpochSeconds(boost::posix_time::ptime& t);
    
    // conversions
    //! converts a hexadecimal color (0xRRGGBB) to a Color
	static ci::Color toColor(uint32_t hex);
	//! converts a hexadecimal color (0xAARRGGBB) to a ColorA
	static ci::ColorA toColorA(uint32_t hex);
	//! converts a string to an integer
	static int toInt(const std::string &str);
	//! converts a string to a double
	static double toDouble(const std::string &str);
	//!
	template<typename T>
	static T wrap(T value, T min, T max) {
		T range = (max - min);
		T frac = ((value - min) / range);
		frac -= ci::math<T>::floor(frac);
        
		return min + (frac * range);
	};
};

#endif
