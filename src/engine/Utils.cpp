//
//  Utils.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 12-02-08.
//  Copyright (c) 2012 ewerx. All rights reserved.
//

#include "Utils.h"
#include "cinder/Filesystem.h"
#include "cinder/Utilities.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace std;

/*static*/ ci::fs::path Utils::getUniquePath( const std::string &path, int padding, const std::string &sep ) 
{
    // expand to resolve ~
	fs::path p = expandPath( fs::path( path ) ); 
	string extension = p.extension().string();
	string stem = p.stem().string();
	fs::path parent_path = p.parent_path();
    
	int count = 0;
    
	do
	{
		p = parent_path / ( stem + sep + leftPaddedString( toString(count), padding ) + extension );
		++count;
	}
    while ( fs::exists( p ) && count < math<int>::pow(10, padding) );
    
	return p;
}

/*static*/ std::string Utils::leftPaddedString( const std::string &input, int padding, const std::string pad )
{
	std::string output(input);
	while ( output.size() < padding )
	{
		output = pad + output;
	}
    
	return output;
}