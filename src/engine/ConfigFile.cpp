//
//  ConfigFile.cpp
//  Oculon
//
//  Created by Ehsan Rezaie on 2014-11-18.
//
//

#include "ConfigFile.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace oculon;

void ConfigFile::load(DataSourceRef source)
{
    try
    {
        mJsonTree = JsonTree(source);
        console() << "[CONFIG] loaded: " << endl << mJsonTree << endl;
    }
    catch (ci::Exception &exc)
    {
        console() << "[CONFIG] failed to parse json: " << exc.what() << endl;
    }
}

Vec2i ConfigFile::getWindowSize()
{
    int width = mJsonTree[kSettings][kRender][kWindow][kWidth].getValue<int>();
    int height = mJsonTree[kSettings][kRender][kWindow][kHeight].getValue<int>();
    
    return Vec2i(width,height);
}

Vec2i ConfigFile::getOutputSize()
{
    int width = mJsonTree[kSettings][kRender][kOutput][kWidth].getValue<int>();
    int height = mJsonTree[kSettings][kRender][kOutput][kHeight].getValue<int>();
    
    return Vec2i(width,height);
}
