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

class Utils
{
public:
    //! Returns a unique pathname, numbered sequentially to avoid conflicts with files of the same/similar name.	
    static ci::fs::path getUniquePath( const std::string &path, const std::string &sep = "_", int padding =3 );

    //! Returns a left-padded string based on the input string
	static std::string leftPaddedString( const std::string &input, int padding=3, const std::string pad="0" );
    
};

#endif
